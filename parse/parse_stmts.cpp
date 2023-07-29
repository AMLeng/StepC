#include "parse.h"
#include "type.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string_view>
#include <map>
namespace parse{
namespace{
std::unique_ptr<ast::CaseStmt> parse_case_stmt(lexer::Lexer& l){
    auto case_keyword = l.get_token();
    if(!token::matches_keyword(case_keyword, "case")){
        throw parse_error::ParseError("Expected keyword \"case\"", case_keyword);
    }
    auto c = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::CaseStmt>(case_keyword, std::move(c), std::move(body));
}
std::unique_ptr<ast::DefaultStmt> parse_default_stmt(lexer::Lexer& l){
    auto default_keyword = l.get_token();
    if(!token::matches_keyword(default_keyword, "default")){
        throw parse_error::ParseError("Expected keyword \"default\"", default_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::DefaultStmt>(default_keyword, std::move(body));
}
std::unique_ptr<ast::SwitchStmt> parse_switch_stmt(lexer::Lexer& l){
    auto switch_keyword = l.get_token();
    if(!token::matches_keyword(switch_keyword, "switch")){
        throw parse_error::ParseError("Expected keyword \"switch\"", switch_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto body = parse_stmt(l);
    return std::make_unique<ast::SwitchStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::WhileStmt> parse_while_stmt(lexer::Lexer& l){
    auto while_keyword = l.get_token();
    if(!token::matches_keyword(while_keyword, "while")){
        throw parse_error::ParseError("Expected keyword \"while\"", while_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto body = parse_stmt(l);
    return std::make_unique<ast::WhileStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::DoStmt> parse_do_stmt(lexer::Lexer& l){
    auto do_keyword = l.get_token();
    if(!token::matches_keyword(do_keyword, "do")){
        throw parse_error::ParseError("Expected keyword \"do\"", do_keyword);
    }
    auto body = parse_stmt(l);
    auto while_keyword = l.get_token();
    if(!token::matches_keyword(while_keyword, "while")){
        throw parse_error::ParseError("Expected keyword \"while\"", while_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::DoStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::ForStmt> parse_for_stmt(lexer::Lexer& l){
    auto for_keyword = l.get_token();
    if(!token::matches_keyword(for_keyword, "for")){
        throw parse_error::ParseError("Expected keyword \"for\"", for_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    //Parse initial clause
    auto init = std::variant<std::monostate,std::unique_ptr<ast::DeclList>,std::unique_ptr<ast::Expr>>{};
    if(!token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
        if(token::matches_type(l.peek_token(),token::TokenType::Keyword)){
            init = parse_decl_list(l);
        }else{
            init = parse_expr(l);
        }
    }

    check_token_type(l.get_token(), token::TokenType::Semicolon);
    //Parse control expr
    static const auto fake_token = token::Token{
        token::TokenType::IntegerLiteral, "1",{-1,-1,-1,-1},"COMPILER GENERATED TOKEN, SOURCE LINE NOT AVAILABLE"};
    std::unique_ptr<ast::Expr> control = std::make_unique<ast::Constant>(fake_token);
    if(!token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
        control = parse_expr(l);
    }

    check_token_type(l.get_token(), token::TokenType::Semicolon);
    //Parse post expr
    auto post = std::optional<std::unique_ptr<ast::Expr>>{std::nullopt};
    if(!token::matches_type(l.peek_token(),token::TokenType::RParen)){
        post = parse_expr(l);
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    //Parse body
    auto body = parse_stmt(l);
    return std::make_unique<ast::ForStmt>(std::move(init), std::move(control), std::move(post), std::move(body));
}
std::unique_ptr<ast::IfStmt> parse_if_stmt(lexer::Lexer& l){
    auto if_keyword = l.get_token();
    if(!token::matches_keyword(if_keyword, "if")){
        throw parse_error::ParseError("Expected keyword \"if\"", if_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto if_condition = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto if_body = parse_stmt(l);
    auto maybe_else= l.peek_token();
    if(maybe_else.type != token::TokenType::Keyword || !token::matches_keyword(maybe_else, "else")){
        return std::make_unique<ast::IfStmt>(std::move(if_condition), std::move(if_body));
    }
    check_token_type(l.get_token(), token::TokenType::Keyword);
    auto else_body = parse_stmt(l);
    return std::make_unique<ast::IfStmt>(std::move(if_condition), std::move(if_body), std::move(else_body));
}
std::unique_ptr<ast::GotoStmt> parse_goto_stmt(lexer::Lexer& l){
    auto goto_keyword = l.get_token();
    check_token_type(goto_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(goto_keyword, "goto")){
        throw parse_error::ParseError("Expected keyword \"goto\"", goto_keyword);
    }
    auto ident_tok = l.get_token();
    check_token_type(ident_tok, token::TokenType::Identifier);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::GotoStmt>(ident_tok);
}
std::unique_ptr<ast::LabeledStmt> parse_labeled_stmt(lexer::Lexer& l){
    auto ident_tok = l.get_token();
    check_token_type(ident_tok, token::TokenType::Identifier);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::LabeledStmt>(ident_tok, std::move(body));
}
std::unique_ptr<ast::BreakStmt> parse_break_stmt(lexer::Lexer& l){
    auto break_keyword = l.get_token();
    check_token_type(break_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(break_keyword, "break")){
        throw parse_error::ParseError("Expected keyword \"break\"", break_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::BreakStmt>(break_keyword);
}
std::unique_ptr<ast::ContinueStmt> parse_continue_stmt(lexer::Lexer& l){
    auto continue_keyword = l.get_token();
    check_token_type(continue_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(continue_keyword, "continue")){
        throw parse_error::ParseError("Expected keyword \"continue\"", continue_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::ContinueStmt>(continue_keyword);
}
std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l){
    auto return_keyword = l.get_token();
    check_token_type(return_keyword, token::TokenType::Keyword);

    if(!token::matches_keyword(return_keyword, "return")){
        throw parse_error::ParseError("Expected keyword \"return\"", return_keyword);
    }
    if(l.peek_token().type == token::TokenType::Semicolon){
        l.get_token();
        return std::make_unique<ast::ReturnStmt>(return_keyword, std::nullopt);
    }
    auto ret_value = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::ReturnStmt>(return_keyword, std::move(ret_value));
}

} //anon namespace

std::unique_ptr<ast::CompoundStmt> parse_compound_stmt(lexer::Lexer& l){
    auto stmt_body = std::vector<std::unique_ptr<ast::BlockItem>>{};
    check_token_type(l.get_token(), token::TokenType::LBrace);
    while(l.peek_token().type != token::TokenType::RBrace){
        stmt_body.push_back(parse_block_item(l));
    }
    check_token_type(l.get_token(), token::TokenType::RBrace);
    return std::make_unique<ast::CompoundStmt>(std::move(stmt_body));
}
std::unique_ptr<ast::Stmt> parse_stmt(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "return")){
        return parse_return_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "if")){
        return parse_if_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "for")){
        return parse_for_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "do")){
        return parse_do_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "while")){
        return parse_while_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "continue")){
        return parse_continue_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "break")){
        return parse_break_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "goto")){
        return parse_goto_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "switch")){
        return parse_switch_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "case")){
        return parse_case_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "default")){
        return parse_default_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && !token::matches_keyword(next_token, "sizeof")){
        throw parse_error::ParseError("Unknown keyword in statement beginning", next_token);
    }
    if(next_token.type == token::TokenType::LBrace){
        return parse_compound_stmt(l);
    }
    if(next_token.type == token::TokenType::Semicolon){
        l.get_token();
        return std::make_unique<ast::NullStmt>();
    }
    if(next_token.type == token::TokenType::Identifier){
        auto maybe_colon = l.peek_token(2);
        if(maybe_colon.type == token::TokenType::Colon){
            return parse_labeled_stmt(l);
        }
    }
    //If is a typedef name will also parse var decl, but that's for later
    auto expr = parse_expr(l);
    auto semicolon = l.get_token();
    check_token_type(semicolon, token::TokenType::Semicolon);
    return std::move(expr);
}
} //namespace parse
