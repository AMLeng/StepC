#ifndef _PARSE_
#define _PARSE_
#include "lexer.h"
#include "ast.h"
#include <memory>
namespace parse{
typedef std::pair<std::optional<token::Token>, type::CType> Declarator;

//in parse.cpp
void check_token_type(const token::Token& tok, token::TokenType type);
std::unique_ptr<ast::Program> construct_ast(lexer::TokenStream& l);
std::unique_ptr<ast::BlockItem> parse_block_item(lexer::TokenStream& l);
std::unique_ptr<ast::AmbiguousBlock> parse_ambiguous_block(lexer::TokenStream& l);

//in parse_exprs.cpp
std::unique_ptr<ast::Expr> parse_expr(lexer::TokenStream& l, int min_bind_power = 0);
std::unique_ptr<ast::Decl> parse_init_decl(lexer::TokenStream& l, Declarator declarator);
//parse_binary_op and parse_variable are made accessible here since they're used in stage 5 testing
std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::TokenStream& l, std::unique_ptr<ast::Expr> left, int min_bind_power);
std::unique_ptr<ast::Variable> parse_variable(lexer::TokenStream& l);

//in parse_stmts.cpp
std::unique_ptr<ast::Stmt> parse_stmt(lexer::TokenStream& l);
std::unique_ptr<ast::CompoundStmt> parse_compound_stmt(lexer::TokenStream& l);

//in parse_specifiers.cpp
std::pair<type::CType,std::vector<std::unique_ptr<ast::TypeDecl>>> parse_specifiers(lexer::TokenStream& l);

//In parse_decl.cpp
Declarator parse_declarator(type::CType type, lexer::TokenStream& l);
std::pair<std::vector<Declarator>,bool> parse_param_list(lexer::TokenStream& l);
std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::TokenStream& l, std::pair<std::vector<Declarator>,bool> params, Declarator function_type);
std::unique_ptr<ast::DeclList> parse_decl_list(lexer::TokenStream& l);
std::unique_ptr<ast::ExtDecl> parse_ext_decl(lexer::TokenStream& l);
} //namespace parse
#endif
