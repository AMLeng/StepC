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
private:
    std::unique_ptr<std::set<std::optional<unsigned long long int>>> switch_cases;
    std::unique_ptr<std::map<std::string,std::optional<token::Token>>> function_labels;
    std::vector<std::unique_ptr<STable>> children;
    std::map<std::string, type::CType> sym_map;
    STable(STable* p) : parent(p), in_loop(p->in_loop), switch_cases(nullptr),function_labels(nullptr) {}
    std::set<std::optional<unsigned long long int>>* get_switch() const;
public:
    STable() = default;
    STable* new_child();
    bool in_switch() const;
    void add_case(std::optional<unsigned long long int> case_val);
    std::unique_ptr<std::set<std::optional<unsigned long long int>>> transfer_switch_table();
    STable* new_switch_scope_child();
    STable* new_function_scope_child();
    std::optional<token::Token> unmatched_label() const;
    void require_label(const token::Token& tok);
    void add_label(const std::string& name);
    void add_symbol(std::string name, type::CType type);
    bool has_symbol(std::string name);
    type::CType symbol_type(std::string name);
};
}//namespace symbol
#endif
