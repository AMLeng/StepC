#include "parse.h"
#include "parse_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
namespace parse{

namespace{
    bool is_literal(const lexer::Token& tok){
        return tok.type == lexer::Token::TokenType::IntegerLiteral;
    }

    bool matches_keyword(const lexer::Token& tok, std::string_view keyword){
        assert(tok.type == lexer::Token::TokenType::Keyword);
        return tok.value == keyword;
    }
    //Check and throw default unexpectd token exception
    void check_token_type(const lexer::Token& tok, lexer::Token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + lexer::Token::string_name(type), tok);
        }
    }

    std::unique_ptr<ast::Constant> parse_constant(lexer::Lexer& l){
        auto constant_value = l.get_token();
        if(!is_literal(constant_value)){
            throw parse_error::ParseError("Expected literal",constant_value);
        }
        return std::make_unique<ast::Constant>(constant_value.value, "int");
    }

    std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l){
        auto return_keyword = l.get_token();
        check_token_type(return_keyword, lexer::Token::TokenType::Keyword);

        if(!matches_keyword(return_keyword, "return")){
            throw parse_error::ParseError("Expected keyword \"return\"", return_keyword);
        }
        auto ret_value = parse_constant(l);
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
