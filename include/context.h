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
    struct FunctionScope : public Scope{
        FunctionScope(type::CType t) : Scope(), ret_type(t),total_locals(0), instructions(1){
            current_depth = 1;
        }
        type::CType ret_type;
        int total_locals;
        int instructions;
    };
    std::map<std::string, std::pair<std::unique_ptr<value::Value>,bool>> global_sym_map;
    std::unique_ptr<FunctionScope> current_function;
    Scope* current_scope;
    std::unique_ptr<basicblock::Block> current_block;
    void enter_block(std::string block_label, std::ostream& output);
    void exit_block(std::ostream& output, std::unique_ptr<basicblock::Terminator> t);
public:
    Context();
    value::Value* prev_temp(int i) const;
    value::Value* new_temp(type::CType t);
    int new_local_name();
    value::Value* add_literal(std::string literal, type::CType type);
    value::Value* add_global(std::string name, type::CType type, bool defined = false);
    value::Value* add_local(std::string name, type::CType type);
    bool has_symbol(std::string name) const;
    value::Value* get_value(std::string name) const;
    void enter_scope();
    void exit_scope();
    void enter_function(type::CType t, std::ostream& output);
    void exit_function(std::ostream& output, std::unique_ptr<basicblock::Terminator> t = nullptr);
    int depth() const;
    type::CType return_type() const;
    void change_block(std::string block_label, std::ostream& output, 
        std::unique_ptr<basicblock::Terminator> old_terminator);
    std::vector<std::string> continue_targets;
    std::vector<std::string> break_targets;
    std::vector<int> switch_numbers;
};
} //namespace context
#endif
