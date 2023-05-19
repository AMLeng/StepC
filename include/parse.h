#ifndef _PARSE_
#define _PARSE_
#include "lexer.h"
#include "ast.h"
#include <memory>
namespace parse{


std::unique_ptr<ast::Constant> parse_constant(lexer::Lexer& l);
std::unique_ptr<ast::UnaryOp> parse_unary_op(lexer::Lexer& l);
std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::Lexer& l, std::unique_ptr<ast::Expr> left, int min_bind_power);
std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power = 0);
std::unique_ptr<ast::Postfix> parse_postfix(lexer::Lexer& l, std::unique_ptr<ast::Expr> arg);
std::unique_ptr<ast::IfStmt> parse_if_stmt(lexer::Lexer& l);
std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l);
std::unique_ptr<ast::CompoundStmt> parse_compound_stmt(lexer::Lexer& l);
std::unique_ptr<ast::ForStmt> parse_for_stmt(lexer::Lexer& l);
std::unique_ptr<ast::DoStmt> parse_do_stmt(lexer::Lexer& l);
std::unique_ptr<ast::CaseStmt> parse_case_stmt(lexer::Lexer& l);
std::unique_ptr<ast::DefaultStmt> parse_default_stmt(lexer::Lexer& l);
std::unique_ptr<ast::SwitchStmt> parse_switch_stmt(lexer::Lexer& l);
std::unique_ptr<ast::WhileStmt> parse_while_stmt(lexer::Lexer& l);
std::unique_ptr<ast::Conditional> parse_conditional(lexer::Lexer& l);
std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l);
std::unique_ptr<ast::ContinueStmt> parse_continue_stmt(lexer::Lexer& l);
std::unique_ptr<ast::BreakStmt> parse_break_stmt(lexer::Lexer& l);
std::unique_ptr<ast::GotoStmt> parse_goto_stmt(lexer::Lexer& l);
std::unique_ptr<ast::LabeledStmt> parse_labeled_stmt(lexer::Lexer& l);
std::unique_ptr<ast::DeclList> parse_decl_list(lexer::Lexer& l);
std::unique_ptr<ast::Variable> parse_variable(lexer::Lexer& l);
std::unique_ptr<ast::LValue> parse_lvalue(lexer::Lexer& l);
std::unique_ptr<ast::Stmt> parse_stmt(lexer::Lexer& l);
std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l);
std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l);

} //namespace parse
#endif
