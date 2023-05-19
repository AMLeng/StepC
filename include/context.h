#ifndef _CONTEXT_
#define _CONTEXT_
#include <string>
#include <memory>
#include <map>
#include <utility>
#include <vector>
#include "value.h"
#include "basic_block.h"
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
    int total_locals;
    int instructions;
    std::unique_ptr<basicblock::Block> current_block;
    void enter_block(std::string block_label, std::ostream& output);
    void exit_block(std::ostream& output, std::unique_ptr<basicblock::Terminator> t);
public:
    Context();
    value::Value* prev_temp(int i) const;
    value::Value* new_temp(type::BasicType t);
    int new_local_name();
    value::Value* add_literal(std::string literal, type::BasicType type);
    value::Value* add_global(std::string name, type::BasicType type);//Not implemented yet
    value::Value* add_local(std::string name, type::BasicType type);
    bool has_symbol(std::string name) const;
    value::Value* get_value(std::string name) const;
    void enter_scope();
    void exit_scope();
    void enter_function(type::BasicType t, std::ostream& output);
    void exit_function(std::ostream& output, std::unique_ptr<basicblock::Terminator> t = nullptr);
    int depth() const;
    type::BasicType return_type() const;
    void change_block(std::string block_label, std::ostream& output, 
        std::unique_ptr<basicblock::Terminator> old_terminator);
    std::vector<std::string> continue_targets;
    std::vector<std::string> break_targets;
};
} //namespace context
#endif
