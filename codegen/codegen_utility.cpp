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

} //namespace

void print_whitespace(int depth, std::ostream& output){
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

value::Value* convert(type::CType original_target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    type::BasicType target_type = std::get<type::BasicType>(original_target_type);
    if(target_type == type::from_str("_Bool")){
        std::string command = std::visit(overloaded{
                [](type::IType){return "icmp ne";},
                [](type::FType){return "fcmp une";},
                }, val->get_type());

        print_whitespace(c.depth(), output);
        auto new_tmp = c.new_temp(type::IType::Bool);
        output << new_tmp->get_value() <<" = "<<command<<" "<<type::ir_type(val->get_type());
        output << std::visit(overloaded{
            [](type::IType){return " 0, ";},
            [](type::FType){return " 0.0, ";},
            }, val->get_type()) << val->get_value() <<std::endl;
        return new_tmp;
    }
    if(type::ir_type(target_type) == type::ir_type(val->get_type())){
        return std::move(val);
    }
    std::string command = std::visit([](auto t, auto s){
        return convert_command(t, s);
    }, target_type, val->get_type());

    print_whitespace(c.depth(), output);
    auto new_tmp = c.new_temp(target_type);
    output << new_tmp->get_value() <<" = " << command <<" " << type::ir_type(val->get_type()) <<" ";
    output << val->get_value() << " to " << type::ir_type(target_type) << std::endl;
    return new_tmp;
}
value::Value* make_command(type::CType t, std::string command, value::Value* left, value::Value* right, 
    std::ostream& output, context::Context& c){
    //Perhaps confusingly, note that the type specified is always the type of the left operand
    print_whitespace(c.depth(), output);
    auto new_temp = c.new_temp(t);
    output << new_temp->get_value()<<" = "<<command<<" "<<type::ir_type(left->get_type())<<" " << left->get_value() <<", "<< right->get_value()<<std::endl;
    return new_temp;
}
value::Value* make_load(value::Value* local, std::ostream& output, context::Context& c){
    auto result = c.new_temp(local->get_type());
    print_whitespace(c.depth(), output);
    output << result->get_value()<<" = load "<<type::ir_type(local->get_type());
    output << ", " <<type::ir_type(local->get_type())<<"* "<<local->get_value()<<std::endl;
    return result;
}
value::Value* make_tmp_reg(type::CType t, std::ostream& output, context::Context& c){
    auto new_tmp = c.new_temp(t);
    print_whitespace(c.depth(), output);
    output << new_tmp->get_value() <<" = alloca "<<type::ir_type(new_tmp->get_type()) <<std::endl;
    return new_tmp;
}
void make_store(value::Value* val, value::Value* reg, std::ostream& output, context::Context& c){
    print_whitespace(c.depth(), output);
    output << "store "<<type::ir_type(val->get_type())<<" "<<val->get_value();
    output<<", "<<type::ir_type(reg->get_type())<<"* "<<reg->get_value()<<std::endl;
}



} //namespace codegen_utility
