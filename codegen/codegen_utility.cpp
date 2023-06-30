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
        auto new_tmp = c.new_temp(target_type);
        output << new_tmp->get_value() <<" = ptrtoint "<<type::ir_type(val->get_type());
        output << " "<<val->get_value()<<" to "<<type::ir_type(new_tmp->get_type())<<std::endl;
        return new_tmp;
    }
    if(!std::holds_alternative<type::BasicType>(val->get_type())){
        assert(false && "Tried to convert non-basic, non-pointer type to basic type");
    }
    auto val_type = std::get<type::BasicType>(val->get_type());
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
        return val; //Do nothing
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
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

std::string default_value(type::CType type){
    return std::visit(type::make_visitor<std::string>(
                [](const type::IType& i){return "0";},
                [](const type::FType& f){return "0.0";},
                [](const type::VoidType& v){return "void";},
                [](const type::PointerType& p)->std::string{return "null";},
                [](const type::FuncType& ){throw std::runtime_error("No default function value");},
                [](const type::ArrayType& ){return "zeroinitializer";}
                ), type);
}
value::Value* convert(type::CType target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    if(!type::can_cast(val->get_type(),target_type)){
        throw std::runtime_error("Tried to convert "+type::to_string(val->get_type())+" to "+type::to_string(target_type));
    }
    return std::visit(type::make_visitor<value::Value*>(
        [&](const type::BasicType& bt){return convert(bt, val, output, c);},
        [&](const type::VoidType& vt){throw std::runtime_error("Unable to convert value to void type");},
        [&](const type::FuncType& ft){throw std::runtime_error("Unable to convert value to function type");},
        [&](const type::PointerType& pt){return convert(pt, val, output, c);},
        [&](const type::ArrayType& at){return convert(at.decay(), val, output, c);}
    ),target_type);
}
value::Value* make_command(type::CType t, std::string command, value::Value* left, value::Value* right, 
    std::ostream& output, context::Context& c){
    //Perhaps confusingly, note that the type specified is always the type of the left operand
    print_whitespace(c.depth(), output);
    auto new_temp = c.new_temp(t);
    output << new_temp->get_value()<<" = "<<command<<" "<<type::ir_type(left->get_type())<<" " << left->get_value() <<", "<< right->get_value()<<std::endl;
    return new_temp;
}
//Basically returns result = *data_pointer
value::Value* make_load(value::Value* data_pointer, std::ostream& output, context::Context& c){
    assert(type::is_type<type::PointerType>(data_pointer->get_type()) && "Can only load from a pointer");
    auto result_type = type::get<type::PointerType>(data_pointer->get_type()).pointed_type();
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
