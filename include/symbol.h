#ifndef _SYMBOL_
#define _SYMBOL_
#include<vector>
#include<memory>
#include<unordered_set>
#include<exception>
#include<map>
#include<set>
#include<optional>
#include "type.h"
#include "token.h"
namespace symbol{
class BlockTable;
class FuncTable;
class GlobalTable;

class STable{
public:
    bool in_loop = false;
protected:
    STable* parent;
    std::vector<std::unique_ptr<STable>> children;
    std::map<std::string, std::pair<type::CType,bool>> sym_map;
    std::unordered_set<std::string> typedefs;
    STable(STable* p) : parent(p), sym_map() {
        if(p && p->in_loop){
            in_loop = true;
        }else{
            in_loop = false;
        }
    }
    STable() = delete;
public:
    virtual ~STable() = default;
    STable& operator=(const STable&) = delete;
    STable(const STable&) = delete;
    STable& operator=(STable&&) = default;
    STable(STable&&) = default;

    STable* most_recent_child();
    virtual bool in_function() const = 0;
    virtual void add_extern_decl(const std::string& name, const type::CType& type) = 0;
    type::CType get_tag(std::string unmangled_tag) const;
    virtual void add_tag(std::string tag, type::TagType type) = 0;
    type::CType mangle_type_or_throw(type::CType type) const;
    virtual std::string mangle_name(std::string name) const noexcept = 0;
    void add_symbol(std::string name, type::CType type, bool has_def = false);
    void add_typedef(std::string name, type::CType type);
    bool has_symbol(std::string name);
    //Returns true if the given symbol is, in the current scope
    //A valid typedef-name, and false otherwise
    bool resolves_to_typedef(std::string name) const;
    type::CType symbol_type(std::string name) const;
};
class GlobalTable : public STable{
    std::map<std::string, type::CType> external_type_map;
    std::map<std::string, type::CType> tags;
public:
    std::map<std::string, int> local_tag_count;
    GlobalTable() : STable(nullptr), external_type_map() {}
    FuncTable* new_function_scope_child(type::CType t);
    bool in_function() const override;
    void add_extern_decl(const std::string& name, const type::CType& type) override;
    void add_tag(std::string tag, type::TagType type) override;
    std::string mangle_name(std::string name) const noexcept override;
};

class BlockTable : public STable{
    GlobalTable* global;
    FuncTable* current_func;
    std::unique_ptr<std::set<std::optional<unsigned long long int>>> switch_cases;
    std::map<std::string, int> tags;
public:
    BlockTable(GlobalTable* global, FuncTable* func, STable* parent) : 
        STable(parent), global(global), current_func(func), switch_cases(nullptr) {}
    type::CType return_type();
    bool in_function() const override;
    void add_extern_decl(const std::string& name, const type::CType& type) override;
    void add_tag(std::string tag, type::TagType type) override;
    std::string mangle_name(std::string name) const noexcept override;
    bool in_switch() const;
    BlockTable* new_switch_scope_child();
    std::set<std::optional<unsigned long long int>>* get_switch() const;
    void require_label(const token::Token& tok);
    void add_label(const std::string& name);
    void add_case(std::optional<unsigned long long int> case_val);
    std::unique_ptr<std::set<std::optional<unsigned long long int>>> transfer_switch_table();
    std::optional<token::Token> unmatched_label() const;
    BlockTable* new_block_scope_child();
};
class FuncTable : public BlockTable{
    friend BlockTable;
    std::map<std::string,std::optional<token::Token>> function_labels;
    type::CType ret_type;
    public:
    FuncTable(GlobalTable* parent, type::CType type) : BlockTable(parent, this, parent), ret_type(type) {}
};
}//namespace symbol
#endif
