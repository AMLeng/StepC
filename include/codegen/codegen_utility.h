#ifndef _CODEGEN_UTILITY_
#define _CODEGEN_UTILITY_
#include <iostream>
#include "value.h"
#include "type.h"
#include "context.h"
#include "token.h"
namespace codegen_utility{
template <class... Ts> struct overloaded : Ts...{using Ts::operator()...;};
template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

value::Value* convert(type::CType original_target_type, value::Value* val, 
        std::ostream& output, context::Context& c);

void print_whitespace(int, std::ostream&);
value::Value* make_command(type::CType t, std::string command, value::Value* left, value::Value* right, 
    std::ostream& output, context::Context& c);

value::Value* make_load(value::Value* local, std::ostream& output, context::Context& c);
value::Value* make_tmp_reg(type::CType t, std::ostream& output, context::Context& c);
void make_store(value::Value* val, value::Value* reg, std::ostream& output, context::Context& c);

value::Value* bin_op_codegen(value::Value* left, value::Value* right, token::TokenType op_type, type::CType result_type,
    std::ostream& output, context::Context& c);


}//namespace codegen_utility
#endif
