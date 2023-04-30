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
//Implemented in ast_sem.cpp and ast_codegen.cpp

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
    std::string type;
    token::Token tok;
    Expr(token::Token tok) : tok(tok){}
};

struct Constant : public Expr{
    std::string literal;
    Constant(const token::Token& tok);
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct UnaryOp : public Expr{
    std::unique_ptr<Expr> arg;
    UnaryOp(token::Token op, std::unique_ptr<Expr> exp);
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct BinaryOp : public Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    BinaryOp(token::Token op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

} //namespace ast
#endif
