#include "ast.h"
#include "context.h"
#include <cassert>
#include <vector>
namespace context{
void Context::enter_block(std::string block_label, std::ostream& output){
    assert(this->current_block == nullptr && "Starting new block without ending current basic block");
    output << block_label<<":"<<std::endl;
    current_block = std::make_unique<basicblock::Block>(block_label);
}
void Context::exit_block(std::ostream& output, std::unique_ptr<basicblock::Terminator> t){
    if(!current_block){
        assert(false && "Tried to exit block when not inside block");
    }
    if(t){
        current_block->add_terminator(std::move(t));
    }
    assert(current_block->has_terminator() && "Tried to exit block with no terminator");
    ast::AST::print_whitespace(this->depth(), output);
    current_block->print_terminator(output);
    current_block = nullptr;
}

Context::Context() : ret_type(nullptr), global_scope(std::make_unique<Scope>()){
    current_scope = global_scope.get();
}
value::Value* Context::prev_temp(int i) const{
    return current_scope->tmp_map.at(current_scope->tmp_map.size() - i - 1).get();
}
int Context::new_local_name(){
    return total_locals++;
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
void Context::enter_function(type::CType t, std::ostream& output){
    ret_type = std::make_unique<type::CType>(t);
    total_locals = 0; 
    enter_block("0",output);
    instructions = 1; //Instruction 0 is the block label
}
void Context::exit_function(std::ostream& output, std::unique_ptr<basicblock::Terminator> t){
    if(t){
        current_block->add_terminator(std::move(t));
    }
    if(!current_block->has_terminator()){
        current_block->add_terminator(std::make_unique<basicblock::Unreachable>());
    }
    exit_block(output, nullptr);
    ret_type = nullptr;
}
int Context::depth() const{
    return current_scope->current_depth;
}
type::CType Context::return_type() const{
    assert(ret_type);
    return *ret_type;
}
void Context::change_block(std::string block_label, std::ostream& output, 
    std::unique_ptr<basicblock::Terminator> old_terminator){
    if(old_terminator || current_block->has_terminator()){
        exit_block(output,std::move(old_terminator));
    }else{
        exit_block(output, std::make_unique<basicblock::UCond_BR>(block_label));
    }
    enter_block(block_label, output);
}
}//namespace context
