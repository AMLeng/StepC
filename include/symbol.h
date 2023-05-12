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
        return sym_map.find(name) != sym_map.end();
    }
    type::BasicType symbol_type(std::string name){
        return sym_map.at(name);
    }
};
}//namespace symbol
#endif
