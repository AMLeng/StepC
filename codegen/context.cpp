#include "context.h"
#include <cassert>
#include <vector>
namespace context{
Context::Context() : ret_type(nullptr), global_scope(std::make_unique<Scope>()){
    current_scope = global_scope.get();
}
value::Value* Context::prev_temp(int i) const{
    return current_scope->tmp_map.at(current_scope->tmp_map.size() - i - 1).get();
}
value::Value* Context::new_temp(type::BasicType t){
    auto new_tmp_ptr = std::make_unique<value::Value>("%"+std::to_string(instructions),t);
    current_scope->tmp_map.push_back(std::move(new_tmp_ptr));
    instructions++;
    return prev_temp(0);
}
value::Value* Context::add_literal(std::string literal, type::BasicType type){
    //Doesn't matter if already present
    current_scope->literal_map.emplace(literal,std::make_unique<value::Value>(literal,type));
    return current_scope->literal_map.at(literal).get();
}
value::Value* Context::add_local(std::string name, type::BasicType type){
    std::string value = "%" + name +"."+std::to_string(total_locals);
    assert(current_scope->sym_map.find(name) == current_scope->sym_map.end() && "Symbol already present in table");
    current_scope->sym_map.emplace(name,std::make_unique<value::Value>(value, type));
    total_locals++;
    return current_scope->sym_map.at(name).get();
}
bool Context::has_symbol(std::string name) const{
    auto p = current_scope;
    while(p != nullptr){
        if(p->sym_map.find(name) != p->sym_map.end()){
            return true;
        }
        p = p->parent;
    }
    return false;
}
value::Value* Context::get_value(std::string name) const{
    auto p = current_scope;
    while(p != nullptr){
        if(p->sym_map.find(name) != p->sym_map.end()){
            return p->sym_map.at(name).get();
        }
        p = p->parent;
    }
    assert(false && "Failed symbol table lookup in context");
    return nullptr;
}
void Context::enter_scope(){
    current_scope = current_scope->new_child();
}
void Context::exit_scope(){
    assert(current_scope->parent != nullptr && "Tried to leave global scope");
    current_scope = current_scope->parent;
    current_scope->children.pop_back();
}
void Context::enter_function(type::BasicType t){
    ret_type = std::make_unique<type::BasicType>(t);
    total_locals = 0; 
    instructions = 1; //1 for the first block name
}
void Context::exit_function(){
    ret_type = nullptr;
}
int Context::depth() const{
    return current_scope->current_depth;
}
type::BasicType Context::return_type() const{
    assert(ret_type);
    return *ret_type;
}
}//namespace context
