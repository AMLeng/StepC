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
struct Decl;
struct Stmt;
struct ExtDecl;
struct Constant;
struct UnaryOp;
struct BinaryOp;

//Implemented in ast_sem.cpp and ast_codegen.cpp

struct AST{
    static void print_whitespace(int depth, std::ostream& output = std::cout);
    virtual void analyze(symbol::STable* st) = 0;
    virtual void pretty_print(int depth) const = 0;
    virtual ~AST() = 0;
    virtual value::Value* codegen(std::ostream& output, context::Context& c) const = 0;
};
struct BlockItem : virtual public AST{
    //Block items can appear in compound statements
    virtual ~BlockItem() = 0;
};
struct Stmt : virtual public BlockItem{
    //Statements are things that can appear in the body of a function
    virtual ~Stmt() = 0;
};
struct AmbiguousBlock : public BlockItem{
    //An ambiguous block item arises when a block item begins with an identifier,
    //since we can't determine without further context if the identifier is a typedef-name
    //or a variable name
    std::unique_ptr<BlockItem> parsed_item = nullptr;
    std::vector<token::Token> unparsed_tokens;
    token::Token ambiguous_ident;
    AmbiguousBlock(std::vector<token::Token> toks) : unparsed_tokens(toks), ambiguous_ident(toks.front()) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct Initializer{
    virtual ~Initializer() = 0;
    virtual void initializer_codegen(value::Value* variable, std::ostream& output, context::Context& c) const = 0;
    virtual void initializer_print(int depth) const = 0;
    virtual void initializer_analyze(type::CType& variable_type, symbol::STable* st) = 0;
    virtual std::string compute_constant(type::CType type) const = 0;
};
struct InitializerList : public Initializer{
    token::Token tok;
    std::vector<std::unique_ptr<Initializer>> initializers;
    InitializerList(token::Token tok, std::vector<std::unique_ptr<Initializer>> inits) : tok(tok), initializers(std::move(inits)) {}
    void initializer_codegen(value::Value* variable, std::ostream& output, context::Context& c) const;
    void initializer_print(int depth) const;
    void initializer_analyze(type::CType& variable_type, symbol::STable* st);
    std::string compute_constant(type::CType type) const override;
};
typedef std::variant<std::monostate,long long int, long double> ConstantExprType;
struct Expr : virtual public Stmt, public Initializer{
    type::CType type;
    bool analyzed = false;
    ConstantExprType constant_value;
    token::Token tok;
    Expr(token::Token tok) : tok(tok), analyzed(){}
    virtual ~Expr() = 0;
    void initializer_codegen(value::Value* variable, std::ostream& output, context::Context& c) const override;
    void initializer_print(int depth) const override;
    void initializer_analyze(type::CType& variable_type, symbol::STable* st) override;
    std::string compute_constant(type::CType type) const override;
};

struct Program : public AST{
    std::vector<std::unique_ptr<ExtDecl>> decls;
    Program(std::vector<std::unique_ptr<ExtDecl>> decls) : decls(std::move(decls)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
    void analyze(){
        auto global_st = symbol::GlobalTable();
        this->analyze(&global_st);
    }
};

struct NullStmt : public Stmt{
    NullStmt(){}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct TypeDecl : virtual public AST {
    //This is a ``fake'' ast node, for passing information from the parsing step
    //To the analysis step
    token::Token tok;
    TypeDecl(token::Token tok) : tok(tok) {}
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct TypedefDecl : public TypeDecl {
    std::string name;
    type::CType type;
    TypedefDecl(token::Token tok, type::CType type) : 
        TypeDecl(tok), name(tok.value), type(std::move(type)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
};
struct TagDecl : public TypeDecl {
    type::TagType type;
    TagDecl(token::Token tok, type::TagType type) : TypeDecl(tok), type(std::move(type)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
};
struct ExtDecl : virtual public AST{
    std::vector<std::unique_ptr<TypeDecl>> tag_decls;
    ExtDecl(std::vector<std::unique_ptr<TypeDecl>> decls) : tag_decls(std::move(decls)) {}
    virtual ~ExtDecl() = 0;
};
struct Decl : virtual public AST{
    const std::string name;
    const token::Token tok;
    type::CType type;
    Decl(token::Token tok, type::CType type) 
        : tok(tok), name(tok.value), type(type) {}
    virtual ~Decl() = 0;
};
struct DeclList : public BlockItem, public ExtDecl{
    bool analyzed = false;
    std::vector<std::unique_ptr<Decl>> decls;
    DeclList(std::vector<std::unique_ptr<Decl>> decls, std::vector<std::unique_ptr<TypeDecl>> tags) 
        : ExtDecl(std::move(tags)), decls(std::move(decls)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct FunctionDecl : public Decl{
    bool analyzed = false;
    FunctionDecl(token::Token name_tok, type::FuncType type) 
        : Decl(name_tok, type){}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct VarDecl : public Decl{
    bool analyzed = false;
    std::optional<std::unique_ptr<Initializer>> assignment;
    //Type qualifiers and storage class specifiers to be implemented later
    VarDecl(token::Token tok, type::CType type,std::optional<std::unique_ptr<Initializer>> assignment = std::nullopt) 
        : Decl(tok,type), assignment(std::move(assignment)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct EnumVarDecl : public TypeDecl{
    std::unique_ptr<Expr> initializer;
    EnumVarDecl(token::Token tok, std::unique_ptr<Expr> initializer)
        : TypeDecl(tok), initializer(std::move(initializer)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct DoStmt : public Stmt{
    std::unique_ptr<Expr> control_expr;
    std::unique_ptr<Stmt> body;
    DoStmt(std::unique_ptr<Expr> control, std::unique_ptr<Stmt> body)
        : control_expr(std::move(control)), body(std::move(body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct WhileStmt : public Stmt{
    std::unique_ptr<Expr> control_expr;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> control, std::unique_ptr<Stmt> body)
        : control_expr(std::move(control)), body(std::move(body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct ForStmt : public Stmt{
    typedef std::variant<std::monostate,std::unique_ptr<DeclList>,std::unique_ptr<Expr>, std::unique_ptr<AmbiguousBlock>> InitClauseTypes;
    InitClauseTypes init_clause;
    std::unique_ptr<Expr> control_expr;
    std::optional<std::unique_ptr<Expr>> post_expr;
    std::unique_ptr<Stmt> body;
    ForStmt(InitClauseTypes init, std::unique_ptr<Expr> control, 
        std::optional<std::unique_ptr<Expr>> post, std::unique_ptr<Stmt> body)
        : init_clause(std::move(init)), control_expr(std::move(control)), 
        post_expr(std::move(post)) , body(std::move(body)){}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct IfStmt : public Stmt{
    std::unique_ptr<Expr> if_condition;
    std::unique_ptr<Stmt> if_body;
    std::optional<std::unique_ptr<Stmt>> else_body;
    IfStmt(std::unique_ptr<Expr> if_condition, std::unique_ptr<Stmt> if_body, std::optional<std::unique_ptr<Stmt>> else_body = std::nullopt) : 
        if_condition(std::move(if_condition)), if_body(std::move(if_body)), else_body(std::move(else_body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct CaseStmt : public Stmt{
    token::Token tok;
    std::unique_ptr<Expr> label;
    std::unique_ptr<Stmt> stmt;
    CaseStmt(token::Token tok, std::unique_ptr<Expr> c, std::unique_ptr<Stmt> stmt) 
        : tok(tok), label(std::move(c)), stmt(std::move(stmt)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct DefaultStmt : public Stmt{
    token::Token tok;
    std::unique_ptr<Stmt> stmt;
    DefaultStmt(token::Token tok, std::unique_ptr<Stmt> stmt) 
        : tok(tok), stmt(std::move(stmt)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct SwitchStmt : public Stmt{
    std::unique_ptr<Expr> control_expr;
    std::unique_ptr<Stmt> switch_body;
    type::BasicType control_type;
    SwitchStmt(std::unique_ptr<Expr> expr, std::unique_ptr<Stmt> body) 
        :control_expr(std::move(expr)), switch_body(std::move(body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
private:
    std::unique_ptr<std::set<std::optional<unsigned long long int>>> case_table;
};
struct CompoundStmt : public Stmt{
    std::vector<std::unique_ptr<BlockItem>> stmt_body;
    CompoundStmt(std::vector<std::unique_ptr<BlockItem>> stmt_body) : stmt_body(std::move(stmt_body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct FunctionDef : public ExtDecl, public FunctionDecl{
    std::vector<std::unique_ptr<VarDecl>> params;
    std::unique_ptr<CompoundStmt> function_body;
    FunctionDef(token::Token tok, type::FuncType type, std::vector<std::unique_ptr<VarDecl>> param_decls, 
        std::unique_ptr<CompoundStmt> body, std::vector<std::unique_ptr<TypeDecl>> tags) : 
        ExtDecl(std::move(tags)), FunctionDecl(tok, type), params(std::move(param_decls)), function_body(std::move(body)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct LabeledStmt : public Stmt{
    token::Token ident_tok;
    std::unique_ptr<Stmt> stmt;
    LabeledStmt(token::Token tok, std::unique_ptr<Stmt> stmt) 
        : ident_tok(tok), stmt(std::move(stmt)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct GotoStmt : public Stmt{
    token::Token ident_tok;
    GotoStmt(token::Token tok) :ident_tok(tok) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct ContinueStmt : public Stmt{
    token::Token tok;
    ContinueStmt(token::Token tok) : tok(tok) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct BreakStmt : public Stmt{
    token::Token tok;
    BreakStmt(token::Token tok) : tok(tok) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct ReturnStmt : public Stmt{
    token::Token tok;
    std::optional<std::unique_ptr<Expr>> return_expr;
    ReturnStmt(token::Token tok, std::optional<std::unique_ptr<Expr>> ret_expr) : tok(tok), return_expr(std::move(ret_expr)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct Conditional : public Expr{
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> true_expr;
    std::unique_ptr<Expr> false_expr;
    Conditional(token::Token op_tok, std::unique_ptr<Expr> cond, std::unique_ptr<Expr> t,std::unique_ptr<Expr> f) :
        Expr(op_tok), cond(std::move(cond)), true_expr(std::move(t)), false_expr(std::move(f)) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct Variable : public Expr{
    std::string variable_name;
    Variable(token::Token tok) : Expr(tok), variable_name(tok.value) {}
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct StrLiteral : public Expr{
    std::string literal;
    StrLiteral(std::vector<token::Token> toks);
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct Constant : public Expr{
    std::string literal;
    Constant(const token::Token& tok);
    void analyze(symbol::STable*) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct MemberAccess : public Expr{
    std::unique_ptr<Expr> arg;
    std::string index;
    MemberAccess(token::Token tok, std::unique_ptr<Expr> argument, std::string index) : 
        Expr(tok), arg(std::move(argument)), index(std::move(index)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct ArrayAccess : public Expr{
    std::unique_ptr<Expr> arg;
    std::unique_ptr<Expr> index;
    ArrayAccess(token::Token tok, std::unique_ptr<Expr> argument, std::unique_ptr<Expr> index) : 
        Expr(tok), arg(std::move(argument)), index(std::move(index)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct Alignof : public Expr{
    std::unique_ptr<Expr> arg;
    Alignof(token::Token tok, std::unique_ptr<Expr> arg) :
        Expr(tok), arg(std::move(arg)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct Sizeof : public Expr{
    std::unique_ptr<Expr> arg;
    Sizeof(token::Token tok, std::unique_ptr<Expr> arg) :
        Expr(tok), arg(std::move(arg)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct FuncCall : public Expr{
    std::unique_ptr<Expr> func;
    std::vector<std::unique_ptr<Expr>> args;
    FuncCall(token::Token tok, std::unique_ptr<Expr> func, std::vector<std::unique_ptr<Expr>> args) : 
        Expr(tok), func(std::move(func)), args(std::move(args)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct Postfix : public Expr{
    std::unique_ptr<Expr> arg;
    Postfix(token::Token op, std::unique_ptr<Expr> exp) : 
        Expr(op), arg(std::move(exp)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
struct UnaryOp : public Expr{
    std::unique_ptr<Expr> arg;
    UnaryOp(token::Token op, std::unique_ptr<Expr> exp) : 
        Expr(op), arg(std::move(exp)) {}
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};

struct BinaryOp : public Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    type::CType new_left_type;
    type::CType new_right_type;
    BinaryOp(token::Token op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : 
        Expr(op), left(std::move(left)), right(std::move(right)) { }
    void analyze(symbol::STable* st) override;
    void pretty_print(int depth) const override;
    value::Value* codegen(std::ostream& output, context::Context& c) const override;
};
bool is_lval(const ast::AST* node);

} //namespace ast
#endif
