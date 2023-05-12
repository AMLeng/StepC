#ifndef _AST_
#define _AST_
#include<memory>
#include<optional>
#include<string>
#include<iostream>
#include<vector>
#include "context.h"
#include "value.h"
#include "token.h"
#include "type.h"
#include "symbol.h"
namespace ast{

//Forward declare node types
struct Program;
struct FunctionDef;
struct ReturnStmt;
struct Expr;
struct Constant;
struct UnaryOp;
struct BinaryOp;

//Implemented in ast_sem.cpp and ast_codegen.cpp

struct AST{
    static void print_whitespace(int depth, std::ostream& output = std::cout);
    virtual void analyze(symbol::STable* st) = 0;
    virtual void pretty_print(int depth) = 0;
    virtual ~AST() = 0;
    virtual value::Value* codegen(std::ostream& output, context::Context& c) = 0;
};

struct Program : public AST{
    std::unique_ptr<FunctionDef> main_method;
    Program(std::unique_ptr<FunctionDef> main) : main_method(std::move(main)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
    void analyze(){
        auto global_st = symbol::STable();
        this->analyze(&global_st);
    }
};

struct Stmt : public AST{
    //Statements are things that can appear in the body of a function
    virtual ~Stmt() = 0;
};

struct Decl : public AST{
    //Declarations are things that can appear at global scope,
    //And which require altering the symbol table
    const std::string name;
    const token::Token tok;
    Decl(token::Token tok) : tok(tok), name(tok.value) {}
    virtual ~Decl() = 0;
};

struct VarDecl : public Decl, public Stmt{
    bool analyzed = false;
    const type::BasicType type;
    std::optional<std::unique_ptr<BinaryOp>> assignment;
    //Type qualifiers and storage class specifiers to be implemented later
    VarDecl(token::Token tok, type::BasicType type,std::optional<std::unique_ptr<BinaryOp>> assignment = std::nullopt) 
        : Decl(tok), type(type), assignment(std::move(assignment)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};
struct IfStmt : public Stmt{
    std::unique_ptr<Expr> if_condition;
    std::unique_ptr<Stmt> if_body;
    std::optional<std::unique_ptr<Stmt>> else_body;
    IfStmt(std::unique_ptr<Expr> if_condition, std::unique_ptr<Stmt> if_body, std::optional<std::unique_ptr<Stmt>> else_body = std::nullopt) : 
        if_condition(std::move(if_condition)), if_body(std::move(if_body)), else_body(std::move(else_body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};
struct CompoundStmt : public Stmt{
    std::vector<std::unique_ptr<Stmt>> stmt_body;
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    CompoundStmt(std::vector<std::unique_ptr<Stmt>> stmt_body) : stmt_body(std::move(stmt_body)) {}
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

struct FunctionDef : public AST{
    std::string name;
    type::BasicType return_type;
    std::unique_ptr<CompoundStmt> function_body;
    FunctionDef(std::string name, type::BasicType ret_type, std::unique_ptr<CompoundStmt> body) : 
        name(name), return_type(ret_type), function_body(std::move(body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth);
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

struct ReturnStmt : public Stmt{
    std::unique_ptr<Expr> return_expr;
    ReturnStmt(std::unique_ptr<Expr> ret_expr) : return_expr(std::move(ret_expr)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth);
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};


struct Expr : public Stmt{
    type::BasicType type;
    bool analyzed = false;
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
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

struct Constant : public Expr{
    std::string literal;
    Constant(const token::Token& tok);
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

struct UnaryOp : public Expr{
    std::unique_ptr<Expr> arg;
    UnaryOp(token::Token op, std::unique_ptr<Expr> exp) : 
        Expr(op), arg(std::move(exp)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

struct BinaryOp : public Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    BinaryOp(token::Token op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : 
        Expr(op), left(std::move(left)), right(std::move(right)) { }
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) override;
    value::Value* codegen(std::ostream& output, context::Context& c) override;
};

} //namespace ast
#endif
