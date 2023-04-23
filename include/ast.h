#include<memory>
#include<string>
#include<iostream>
#include "context.h"
#include "value.h"
#include "token.h"
#ifndef _AST_
#define _AST_
namespace ast{

class AST;
//List of node types
class Program;
class FunctionDef;
class ReturnStmt;
class Expr;
class Constant;
class UnaryOp;

typedef std::string Type;

struct AST{
    virtual void pretty_print(int depth) = 0;
    static void print_whitespace(int depth, std::ostream& output = std::cout);
    virtual std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) = 0;
};

struct Program : public AST{
    std::unique_ptr<FunctionDef> main_method;
    Program(std::unique_ptr<FunctionDef> main) : main_method(std::move(main)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct FunctionDef : public AST{
    std::string name;
    Type return_type;
    std::unique_ptr<ReturnStmt> function_body;
    FunctionDef(std::string name, Type ret_type, std::unique_ptr<ReturnStmt> ret) : name(name), return_type(ret_type), function_body(std::move(ret)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct ReturnStmt : public AST{
    std::unique_ptr<Expr> return_expr;
    ReturnStmt(std::unique_ptr<Expr> ret_expr) : return_expr(std::move(ret_expr)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct Expr : public AST{
};

struct Constant : public Expr{
    std::string literal;
    std::string type;
    Constant(token::Token tok);
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct UnaryOp : public Expr{
    std::unique_ptr<Expr> arg;
    token::TokenType op;
    UnaryOp(token::TokenType op_name, std::unique_ptr<Expr> exp) : 
        Expr(), op(op_name), arg(std::move(exp)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct BinaryOp : public Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    token::TokenType op;
    BinaryOp(token::TokenType op_name, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : 
        Expr(), op(op_name), left(std::move(left)), right(std::move(right)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

} //namespace ast
#endif
