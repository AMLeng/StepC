#ifndef _CONTEXT_
#define _CONTEXT_
#include <string>
#include <cassert>
#include <memory>
#include <map>
#include <utility>
#include "value.h"
namespace context{
class Context{
    struct Scope{
        int current_depth;
        Scope* parent;
        std::vector<std::unique_ptr<Scope>> children;
        //Maps a name to pair <ir_type, value>
        std::map<std::string, std::unique_ptr<value::Value>> sym_map;
        std::vector<std::unique_ptr<value::Value>> tmp_map;
        std::map<std::string, std::unique_ptr<value::Value>> literal_map;
        Scope(Scope* p) : parent(p) {
            assert(p && "Tried to create scope with null parent");
            current_depth = p->current_depth + 1;
        }
        Scope() = default;
        Scope* new_child(){
            auto child = Scope(this);
            children.push_back(std::make_unique<Scope>(std::move(child)));
            return children.back().get();
        }
    };
    std::unique_ptr<type::BasicType> ret_type;
    const std::unique_ptr<Scope> global_scope;
    Scope* current_scope;
public:
    Context() : ret_type(nullptr), global_scope(std::make_unique<Scope>()){
        current_scope = global_scope.get();
    }
    value::Value* prev_temp(int i) const{
        return current_scope->tmp_map.at(current_scope->tmp_map.size() - i - 1).get();
    }
    value::Value* new_temp(type::BasicType t){
        auto new_tmp_ptr = std::make_unique<value::Value>("%"+std::to_string(current_scope->tmp_map.size() + 1),t);
        current_scope->tmp_map.push_back(std::move(new_tmp_ptr));
        return prev_temp(0);
    }
    value::Value* add_literal(std::string literal, type::BasicType type){
        //Doesn't matter if already present
        current_scope->literal_map.emplace(literal,std::make_unique<value::Value>(literal,type));
        return current_scope->literal_map.at(literal).get();
    }
    value::Value* add_local(std::string name, type::BasicType type, std::string value){
        assert(current_scope->sym_map.find(name) == current_scope->sym_map.end() && "Symbol already present in table");
        current_scope->sym_map.emplace(name,std::make_unique<value::Value>(value, type));
        return current_scope->sym_map.at(name).get();
    }
    bool has_symbol(std::string name){
        return current_scope->sym_map.find(name) != current_scope->sym_map.end();
    }
    value::Value* get_type(std::string name){
        return current_scope->sym_map.at(name).get();
    }
    void enter_function(type::BasicType t){
        current_scope = current_scope->new_child();
        ret_type = std::make_unique<type::BasicType>(t);
    }
    void exit_function(){
        current_scope = current_scope->parent;
        current_scope->children.pop_back();
        ret_type = nullptr;
    }
    int depth() const{
        return current_scope->current_depth;
    }
    type::BasicType return_type(){
        assert(ret_type);
        return *ret_type;
    }
};
} //namespace context
#endif
