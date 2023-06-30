#include "basic_block.h"
#include "codegen/codegen_utility.h"

namespace basicblock{
Terminator::~Terminator (){}
std::string RET::get_instruction(){
    if(ret_val){
        return "ret "+type::ir_type(ret_val->get_type())+" "+ret_val->get_value();
    }else{
        return "ret void";
    }
}
std::string DefaultRet::get_instruction(){
    if(type::is_type<type::ArrayType>(type)){
        throw std::runtime_error("Cannot have array type as return value");
    }
    if(type::is_type<type::VoidType>(type)){
        return "ret void";
    }else{
        return "ret "+type::ir_type(type)+" "+codegen_utility::default_value(type);
    }
}
Cond_BR::Cond_BR(value::Value* cond, std::string tl, std::string fl) :
    cond(cond), t_label(tl), f_label(fl){
    assert(cond && "Tried to create conditional branch with no condition");
    assert(type::ir_type(cond->get_type())=="i1" 
        && "Tried to create conditional branch with non bool condition");
}
std::string Cond_BR::get_instruction() {
    auto ss = std::stringstream{};
    ss << "br "<<type::ir_type(cond->get_type())<<" "<<cond->get_value()<<", label %";
    ss << t_label <<", label %"<<f_label;
    return ss.str();
}
std::string UCond_BR::get_instruction() {
    return "br label %"+label;
}
std::string Unreachable::get_instruction() {
    return "unreachable";
}
std::string Block::get_label(){
    return label;
}
bool Block::has_terminator() const{
    return t != nullptr;
}
void Block::add_terminator(std::unique_ptr<Terminator> term){
    t = std::move(term);
}
void Block::print_terminator(std::ostream& output) const{
    assert(t && "No terminator to print");
    output << t->get_instruction() <<std::endl;
}

}//namespace basicblock
