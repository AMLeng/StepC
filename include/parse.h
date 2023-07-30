#ifndef _PARSE_
#define _PARSE_
#include "lexer.h"
#include "ast.h"
#include <memory>
namespace parse{
typedef std::pair<std::optional<token::Token>, type::CType> Declarator;

//in parse.cpp
void check_token_type(const token::Token& tok, token::TokenType type);
std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l);
std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l);

//in parse_exprs.cpp
std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power = 0);
std::unique_ptr<ast::Decl> parse_init_decl(lexer::Lexer& l, Declarator declarator);
//parse_binary_op and parse_variable are made accessible here since they're used in stage 5 testing
std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::Lexer& l, std::unique_ptr<ast::Expr> left, int min_bind_power);
std::unique_ptr<ast::Variable> parse_variable(lexer::Lexer& l);

//in parse_stmts.cpp
std::unique_ptr<ast::Stmt> parse_stmt(lexer::Lexer& l);
std::unique_ptr<ast::CompoundStmt> parse_compound_stmt(lexer::Lexer& l);

//in parse_specifiers.cpp
std::pair<type::CType,std::vector<std::unique_ptr<ast::TypeDecl>>> parse_specifiers(lexer::Lexer& l);

//In parse_decl.cpp
Declarator parse_declarator(type::CType type, lexer::Lexer& l);
std::pair<std::vector<Declarator>,bool> parse_param_list(lexer::Lexer& l);
std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l, std::pair<std::vector<Declarator>,bool> params, Declarator function_type);
std::unique_ptr<ast::DeclList> parse_decl_list(lexer::Lexer& l);
std::unique_ptr<ast::ExtDecl> parse_ext_decl(lexer::Lexer& l);
} //namespace parse
#endif
