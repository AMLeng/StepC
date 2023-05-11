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
    auto new_tmp_ptr = std::make_unique<value::Value>("%"+std::to_string(current_scope->tmp_map.size() + 1),t);
    current_scope->tmp_map.push_back(std::move(new_tmp_ptr));
    return prev_temp(0);
}
value::Value* Context::add_literal(std::string literal, type::BasicType type){
    //Doesn't matter if already present
    current_scope->literal_map.emplace(literal,std::make_unique<value::Value>(literal,type));
    return current_scope->literal_map.at(literal).get();
}
value::Value* Context::add_local(std::string name, type::BasicType type){
    std::string value = "%" + name;
    assert(current_scope->sym_map.find(name) == current_scope->sym_map.end() && "Symbol already present in table");
    current_scope->sym_map.emplace(name,std::make_unique<value::Value>(value, type));
    return current_scope->sym_map.at(name).get();
}
bool Context::has_symbol(std::string name) const{
    return current_scope->sym_map.find(name) != current_scope->sym_map.end();
}
value::Value* Context::get_value(std::string name) const{
    return current_scope->sym_map.at(name).get();
}
void Context::enter_function(type::BasicType t){
    current_scope = current_scope->new_child();
    ret_type = std::make_unique<type::BasicType>(t);
}
void Context::exit_function(){
    current_scope = current_scope->parent;
    current_scope->children.pop_back();
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
