#include<memory>
#include<optional>
#include<string>
#include<iostream>
#include<vector>
#include "context.h"
#include "value.h"
#include "token.h"
#include "type.h"
#ifndef _AST_
#define _AST_
namespace ast{

//Forward declare node types
struct Program;
struct FunctionDef;
struct ReturnStmt;
struct Expr;
struct Constant;
struct UnaryOp;
struct Assign;

//Implemented in ast_sem.cpp and ast_codegen.cpp

struct AST{
    virtual void pretty_print(int depth) = 0;
    static void print_whitespace(int depth, std::ostream& output = std::cout);
    virtual std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) = 0;
    virtual ~AST() = 0;
};

struct Program : public AST{
    std::unique_ptr<FunctionDef> main_method;
    Program(std::unique_ptr<FunctionDef> main) : main_method(std::move(main)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct Stmt : public AST{
    //Statements are things that can appear in the body of a function
    virtual ~Stmt() = 0;
};

struct Decl : public AST{
    //Declarations are things that can appear at global scope,
    //And which require altering the symbol table
    const std::string name;
    Decl(std::string name) : name(name) {}
    virtual ~Decl() = 0;
};

struct VarDecl : public Decl, public Stmt{
    const type::BasicType type;
    std::optional<std::unique_ptr<Assign>> assignment;
    //Type qualifiers and storage class specifiers to be implemented later
    VarDecl(std::string name, type::BasicType type,std::optional<std::unique_ptr<Assign>> assignment = std::nullopt) 
        : Decl(name), type(type), assignment(std::move(assignment)) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct FunctionDef : public AST{
    std::string name;
    type::BasicType return_type;
    std::vector<std::unique_ptr<Stmt>> function_body;
    FunctionDef(std::string name, type::BasicType ret_type, std::vector<std::unique_ptr<Stmt>> body) : 
        name(name), return_type(ret_type), function_body(std::move(body)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct ReturnStmt : public Stmt{
    std::unique_ptr<Expr> return_expr;
    ReturnStmt(std::unique_ptr<Expr> ret_expr) : return_expr(std::move(ret_expr)) {}
    void pretty_print(int depth);
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};


struct Expr : public Stmt{
    type::BasicType type;
    token::Token tok;
    Expr(token::Token tok) : tok(tok){}
    virtual ~Expr() = 0;
};
struct LValue : public Expr{
    LValue(token::Token tok) : Expr(tok){}
    virtual ~LValue() = 0;
};

struct Variable : public LValue{
    std::string variable_name;
    Variable(token::Token tok) : LValue(tok), variable_name(tok.value) {}
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
};

struct Assign : public Expr{
    std::unique_ptr<LValue> left;
    std::unique_ptr<Expr> right;
    void pretty_print(int depth) override;
    std::unique_ptr<value::Value> codegen(std::ostream& output, context::Context& c) override;
    Assign(token::Token tok, std::unique_ptr<LValue> left, std::unique_ptr<Expr> right) : Expr(tok), left(std::move(left)), right(std::move(right)) {}
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
