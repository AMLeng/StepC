#include<memory>
#include<string>
#include<iostream>
#include "context.h"
#include "value.h"
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

class AST{
public:
    virtual void pretty_print(int depth) = 0;
    static void print_whitespace(int depth);
    virtual std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) = 0;
};

class Program : public AST{
    std::unique_ptr<FunctionDef> main_method;
public:
    Program(std::unique_ptr<FunctionDef> main) : main_method(std::move(main)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

class FunctionDef : public AST{
    std::string name;
    Type return_type;
    std::unique_ptr<ReturnStmt> function_body;
public:
    FunctionDef(std::string name, Type ret_type, std::unique_ptr<ReturnStmt> ret) : name(name), return_type(ret_type), function_body(std::move(ret)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

class ReturnStmt : public AST{
    std::unique_ptr<Expr> return_expr;
public:
    ReturnStmt(std::unique_ptr<Expr> ret_expr) : return_expr(std::move(ret_expr)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

class Expr : public AST{
public:
};

class Constant : public Expr{
public:
    std::string literal;
    Constant(std::string literal) : literal(literal) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

class UnaryOp : public Expr{
    std::unique_ptr<Expr> arg;
public:
    std::string op;
    UnaryOp(std::string op_name, std::unique_ptr<Expr> exp) : 
        Expr(), op(op_name), arg(std::move(exp)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

class BinaryOp : public Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
public:
    std::string op;
    BinaryOp(std::string op_name, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : 
        Expr(), op(op_name), left(std::move(left)), right(std::move(right)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

} //namespace ast
#endif
