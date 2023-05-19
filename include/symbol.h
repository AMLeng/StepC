#ifndef _SYMBOL_
#define _SYMBOL_
#include<vector>
#include<memory>
#include<exception>
#include<map>
#include<set>
#include<optional>
#include "type.h"
#include "token.h"
namespace symbol{
class STable{
    STable* parent;
public:
    bool in_loop = false;
    bool in_switch = false;
private:
    std::unique_ptr<std::map<std::string,std::optional<token::Token>>> function_labels;
    std::vector<std::unique_ptr<STable>> children;
    std::map<std::string, type::BasicType> sym_map;
    STable(STable* p) : parent(p), in_loop(p->in_loop), in_switch(p->in_switch),function_labels(nullptr) {}
public:
    STable() = default;
    STable* new_child(){
        auto child = STable(this);
        children.push_back(std::make_unique<STable>(std::move(child)));
        return children.back().get();
    }
    STable* new_function_scope_child(){
        auto child = new_child();
        //Check that we're not already in a function
        auto to_check = child;
        do{
            to_check = to_check->parent;
            if(to_check->function_labels != nullptr){
                throw std::runtime_error("Can't define function inside another function");
            }
        }while(to_check->parent != nullptr);
        child->function_labels = std::make_unique<std::map<std::string,std::optional<token::Token>>>();
        return child;
    }
    std::optional<token::Token> unmatched_label() const{
        if(function_labels == nullptr){
            return parent->unmatched_label();
        }else{
            for(const auto& map_pair : *function_labels){
                if(map_pair.second){
                    return map_pair.second;
                }
            }
            return std::nullopt;
        }
    }
    void require_label(const token::Token& tok){
        if(function_labels == nullptr){
            if(parent == nullptr){
                throw std::runtime_error("Can't have label outside of function");
            }
            parent->require_label(tok);
        }else{
            if(function_labels->find(tok.value) == function_labels->end()){
                function_labels->emplace(tok.value,tok);
            }
        }
    }
    void add_label(const std::string& name){
        if(function_labels == nullptr){
            if(parent == nullptr){
                throw std::runtime_error("Can't have label outside of function");
            }
            parent->add_label(name);
        }else{
            if(function_labels->find(name) != function_labels->end() && function_labels->at(name) == std::nullopt){
                throw std::runtime_error("Label "+name+" already present in symbol table for this function");
            }
            function_labels->insert_or_assign(name,std::nullopt);
        }
    }
    void add_symbol(std::string name, type::BasicType type){
        if(sym_map.find(name) != sym_map.end()){
            throw std::runtime_error("Symbol "+name+" already present in symbol table");
        }
        sym_map.emplace(name,type);
    }
    bool has_symbol(std::string name){
        STable* to_search = this;
        while(to_search != nullptr){
            if(to_search->sym_map.find(name) != to_search->sym_map.end()){
                return true;
            }
            to_search = to_search->parent;
        }
        return false;
    }
    type::BasicType symbol_type(std::string name){
        STable* to_search = this;
        while(to_search != nullptr){
            if(to_search->sym_map.find(name) != to_search->sym_map.end()){
                return to_search->sym_map.at(name);
            }
            to_search = to_search->parent;
        }
        throw std::runtime_error("Symbol "+name+" not found in symbol table");
        __builtin_unreachable();
    }
};
}//namespace symbol
#endif
