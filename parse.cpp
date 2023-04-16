#include "parse.h"
#include "parse_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
namespace parse{

namespace{
    //Declarations
    std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l);

    //Definitions for helper methods
    bool matches_type(const lexer::Token& tok){
        return false;
    }

    template <typename... TokTypes>
    bool matches_type(const lexer::Token& tok, lexer::Token::TokenType t, TokTypes... types){
        return tok.type == t || matches_type(tok, types...);
    }

    bool is_literal(const lexer::Token& tok){
        return matches_type(tok,lexer::Token::TokenType::IntegerLiteral);
    }

    bool matches_keyword(const lexer::Token& tok){
        assert(tok.type == lexer::Token::TokenType::Keyword);
        return false;
    }

    template<typename... Ts>
    bool matches_keyword(const lexer::Token& tok, std::string_view keyword, Ts... ts){
        return tok.value == keyword || matches_keyword(tok, ts...);
    }

    //Check and throw default unexpectd token exception
    void check_token_type(const lexer::Token& tok, lexer::Token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + lexer::Token::string_name(type), tok);
        }
    }

    //Definitions for parsing methods
    std::unique_ptr<ast::Constant> parse_constant(lexer::Lexer& l){
        auto constant_value = l.get_token();
        if(!is_literal(constant_value)){
            throw parse_error::ParseError("Expected literal",constant_value);
        }
        return std::make_unique<ast::Constant>(constant_value.value);
    }
    
    std::unique_ptr<ast::UnaryOp> parse_unary_op(lexer::Lexer& l){
        auto op_token = l.get_token();
        if(!matches_type(op_token,
            lexer::Token::TokenType::Minus,
            lexer::Token::TokenType::BitwiseNot,
            lexer::Token::TokenType::Not)){
            throw parse_error::ParseError("Not valid unary operator",op_token);
        }
        auto expr = parse_expr(l);
        return std::make_unique<ast::UnaryOp>(op_token.value,std::move(expr));
    }

    std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l){
        auto expr_start = l.peek_token();
        if(is_literal(expr_start)){
            return parse_constant(l);
        }
        return parse_unary_op(l);
    }

    std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l){
        auto return_keyword = l.get_token();
        check_token_type(return_keyword, lexer::Token::TokenType::Keyword);

        if(!matches_keyword(return_keyword, "return")){
            throw parse_error::ParseError("Expected keyword \"return\"", return_keyword);
        }
        auto ret_value = parse_expr(l);
        auto semicolon = l.get_token();
        check_token_type(semicolon, lexer::Token::TokenType::Semicolon);
        return std::make_unique<ast::ReturnStmt>(std::move(ret_value));
    }

    std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l){
        auto ret_type = l.get_token();
        if(ret_type.type != lexer::Token::TokenType::Keyword && ret_type.type != lexer::Token::TokenType::Identifier){
            throw parse_error::ParseError("Invalid function return type", ret_type);
        }
        auto name = l.get_token();
        if(name.type != lexer::Token::TokenType::Identifier){
            throw parse_error::ParseError("Invalid function name", name);
        }

        check_token_type(l.get_token(), lexer::Token::TokenType::LParen);
        check_token_type(l.get_token(), lexer::Token::TokenType::RParen);
        check_token_type(l.get_token(), lexer::Token::TokenType::LBrace);

        auto function_body = parse_return_stmt(l);

        check_token_type(l.get_token(), lexer::Token::TokenType::RBrace);
        return std::make_unique<ast::FunctionDef>(name.value, ret_type.value, std::move(function_body));
    }
}//namespace

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    auto main_method = parse_function_def(l);
    return std::make_unique<ast::Program>(std::move(main_method));
}

} //namespace parse
