#ifndef _SYMBOL_
#define _SYMBOL_
#include<vector>
#include<memory>
#include<exception>
#include<map>
#include "type.h"
namespace symbol{
class STable{
    STable* parent;
    std::vector<std::unique_ptr<STable>> children;
    std::map<std::string, type::BasicType> sym_map;
    STable(STable* p) : parent(p) {}
public:
    STable() = default;
    STable* new_child(){
        auto child = STable(this);
        children.push_back(std::make_unique<STable>(std::move(child)));
        return children.back().get();
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
