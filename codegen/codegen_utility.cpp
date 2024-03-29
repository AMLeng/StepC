#include "codegen/codegen_utility.h"
namespace codegen_utility{

namespace{
std::string convert_command(type::IType target, type::IType source){
    if(type::can_represent(target, source)){
        if(type::is_signed_int(source)){
            return "sext";
        }else{
            return "zext";
        }
    }else{
        return "trunc";
    }
}
std::string convert_command(type::IType target, type::FType source){
    if(type::is_signed_int(target)){
        //IF IT CAN'T FIT, POTENTIAL SOURCE OF Undefined Behavior!!!!!!!!!
        return "fptosi";
    }else{
        //IF IT CAN'T FIT, POTENTIAL SOURCE OF UB!!!!!!!!!
        return "fptoui";
    }
}

std::string convert_command(type::FType target, type::IType source){
    if(type::is_signed_int(source)){
        return "sitofp";
    }else{
        return "uitofp";
    }
}
std::string convert_command(type::FType target, type::FType source){
    if(type::can_represent(target, source)){
        return "fpext";
    }else{
        return "fptrunc";
    }
}
value::Value* convert(type::BasicType target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    if(type::is_type<type::PointerType>(val->get_type())){
        if(type::is_type<type::FType>(target_type)){
            assert(false && "Tried to convert pointer to float");
        }
        if(target_type == type::BasicType(type::IType::Bool)){
            auto first_stage = convert(type::IType::LLong, val, output, c);
            return convert(type::IType::Bool, first_stage, output, c);
        }else{
            auto new_tmp = c.new_temp(target_type);
            print_whitespace(c.depth(), output);
            output << new_tmp->get_value() <<" = ptrtoint "<<type::ir_type(val->get_type());
            output << " "<<val->get_value()<<" to "<<type::ir_type(new_tmp->get_type())<<std::endl;
            return new_tmp;
        }
    }
    if(!type::is_type<type::BasicType>(val->get_type())){
        assert(false && "Tried to convert non-basic, non-pointer type to basic type");
    }
    auto val_type = type::get<type::BasicType>(val->get_type());
    if(target_type == type::from_str("_Bool")){
        std::string command = std::visit(overloaded{
                [](type::IType){return "icmp ne";},
                [](type::FType){return "fcmp une";},
                }, val_type);

        print_whitespace(c.depth(), output);
        auto new_tmp = c.new_temp(type::IType::Bool);
        output << new_tmp->get_value() <<" = "<<command<<" "<<type::ir_type(val_type);
        output << std::visit(overloaded{
            [](type::IType){return " 0, ";},
            [](type::FType){return " 0.0, ";},
            }, val_type) << val->get_value() <<std::endl;
        return new_tmp;
    }
    if(type::ir_type(target_type) == type::ir_type(val_type)){
        return std::move(val);
    }
    std::string command = std::visit([](auto t, auto s){
        return convert_command(t, s);
    }, target_type, val_type);

    print_whitespace(c.depth(), output);
    auto new_tmp = c.new_temp(target_type);
    output << new_tmp->get_value() <<" = " << command <<" " << type::ir_type(val_type) <<" ";
    output << val->get_value() << " to " << type::ir_type(target_type) << std::endl;
    return new_tmp;
}
value::Value* convert(type::PointerType target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    if(type::is_type<type::PointerType>(val->get_type())){
        //We don't change the value, but we must alter the type of the pointer appropriately for our own type checking
        //Even if all pointers are opaque in the IR itself
        return c.ptr_cast(val, target_type);
    }else{
        if(!type::is_type<type::IType>(val->get_type())){
            assert(false && "Tried to convert non-ptr, non-int type to ptr");
        }
        auto new_tmp = c.new_temp(target_type);
        output << new_tmp->get_value() <<" = inttoptr "<<type::ir_type(val->get_type());
        output << " "<<val->get_value()<<" to "<<type::ir_type(new_tmp->get_type())<<std::endl;
        return new_tmp;
    }
}

} //namespace

void print_whitespace(int depth, std::ostream& output){
    if(depth > 0){
        output << "  ";
    }
}
std::string default_value(type::CType type){
    return type::visit(type::make_visitor<std::string>(
                [](const type::IType& i){return "0";},
                [](const type::FType& f){return "0.0";},
                [](const type::VoidType& v){return "void";},
                [](const type::PointerType& p){return "null";},
                [](const type::FuncType& ){throw std::runtime_error("No default function value");},
                [](const type::ArrayType& ){return "zeroinitializer";},
                [](const type::StructType& ){return "zeroinitializer";},
                [](const type::UnionType& ){return "zeroinitializer";},
                [](const type::UnevaluatedTypedef& ){return "zeroinitializer";}
                ), type);
}
value::Value* convert(type::CType target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    if(!type::can_cast(val->get_type(),target_type)){
        throw std::runtime_error("Tried to convert "+type::to_string(val->get_type())+" to "+type::to_string(target_type));
    }
    if(val->get_type() == target_type){
        return val;
    }
    return type::visit(type::make_visitor<value::Value*>(
    //make_visitor will take the target type and go all the way down to either
    //A specific derived type (distinguishing between array and pointer types), IType, FType, or VoidType
    //So we would need separate IType and FType options to avoid IType and FType binding to auto
    //And we would need a separate option for array types no matter what (to avoid it binding to auto
        [&](type::BasicType bt){return convert(bt, val, output, c);},
        [&](type::ArrayType pt){return convert(type::PointerType(type::CType(pt)), val, output, c);},
        [&](type::PointerType pt){return convert(pt, val, output, c);},
        [](type::VoidType t){throw std::runtime_error("Unable to convert value to given type "+type::to_string(t));},
        [](type::StructType t){throw std::runtime_error("Unable to convert value to given type "+type::to_string(t));},
        [&](type::UnionType t){
            throw std::runtime_error("Unable to convert value to given type "+type::to_string(t));
        },
        [](type::UnevaluatedTypedef t){throw std::runtime_error("Unable to convert value to given type "+type::to_string(t));},
        [](type::FuncType t){throw std::runtime_error("Unable to convert value to given type "+type::to_string(t));}
    ),target_type);
}
value::Value* make_command(type::CType t, std::string command, value::Value* left, value::Value* right, 
    std::ostream& output, context::Context& c){
    //Perhaps confusingly, note that the type specified is always the type of the left operand
    auto lt = left->get_type();
    if(type::is_type<type::ArrayType>(lt)){
        lt = type::get<type::ArrayType>(lt).pointed_type();
    }
    print_whitespace(c.depth(), output);
    auto new_temp = c.new_temp(t);
    output << new_temp->get_value()<<" = "<<command<<" "<<type::ir_type(lt)<<" " << left->get_value() <<", "<< right->get_value()<<std::endl;
    return new_temp;
}
//Basically returns result = *data_pointer
value::Value* make_load(value::Value* data_pointer, std::ostream& output, context::Context& c){
    assert(type::is_type<type::PointerType>(data_pointer->get_type()) && "Can only load from a pointer");
    auto result_type = type::get<type::PointerType>(data_pointer->get_type()).element_type();
    //Loads the pointed type, unless it's an array in which case we get the element instead
    if(type::is_type<type::FuncType>(result_type)){
        //Loading from a function pointer just gives back the function pointer
        return data_pointer;
    }
    auto result = c.new_temp(result_type);
    print_whitespace(c.depth(), output);
    output << result->get_value()<<" = load "<<type::ir_type(result->get_type());
    output << ", " <<type::ir_type(data_pointer->get_type())<<" "<<data_pointer->get_value()<<std::endl;
    return result;
}
value::Value* make_tmp_alloca(type::CType t, std::ostream& output, context::Context& c){
    auto new_tmp = c.new_temp(type::PointerType(t));
    print_whitespace(c.depth(), output);
    output << new_tmp->get_value() <<" = alloca "<<type::ir_type(t) <<std::endl;
    return new_tmp;
}
//Basically performs *mem = data_pointer
void make_store(value::Value* val, value::Value* mem, std::ostream& output, context::Context& c){
    assert(type::is_type<type::PointerType>(mem->get_type()) && "Can only store to a pointer");
    print_whitespace(c.depth(), output);
    output << "store "<<type::ir_type(val->get_type())<<" "<<val->get_value();
    output<<", "<<type::ir_type(mem->get_type())<<" "<<mem->get_value()<<std::endl;
}



} //namespace codegen_utility
