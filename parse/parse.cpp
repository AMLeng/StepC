#include "parse.h"
#include "parse_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
#include <map>
namespace parse{

namespace{
    //Declarations
    std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power = 0);

    //Operator Precedence
    //Unary operators should bind more tightly than any normal binary op
    constexpr int unary_op_binding_power = 10;
    //left binding is less than right binding for left associativity (+, -)
    //and vice versa for right associativity
    std::map<token::TokenType, std::pair<int, int>> binary_op_binding_power = {{
        {token::TokenType::Plus,{1,2}}, {token::TokenType::Minus,{1,2}}, 
        {token::TokenType::Mult, {3,4}}, {token::TokenType::Div, {3,4}}
    }};

    //Definitions for helper methods
    bool matches_type(const token::Token& tok){
        return false;
    }

    template <typename... TokTypes>
    bool matches_type(const token::Token& tok, token::TokenType t, TokTypes... types){
        return tok.type == t || matches_type(tok, types...);
    }

    bool matches_keyword(const token::Token& tok){
        assert(tok.type == token::TokenType::Keyword);
        return false;
    }

    template<typename... Ts>
    bool matches_keyword(const token::Token& tok, std::string_view keyword, Ts... ts){
        return tok.value == keyword || matches_keyword(tok, ts...);
    }

    //Check and throw default unexpected token exception
    void check_token_type(const token::Token& tok, token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + token::string_name(type), tok);
        }
    }


    //Definitions for parsing methods
    std::unique_ptr<ast::Constant> parse_constant(lexer::Lexer& l){
        auto constant_value = l.get_token();
        if(!matches_type(constant_value, 
            token::TokenType::IntegerLiteral, 
            token::TokenType::FloatLiteral)){
            throw parse_error::ParseError("Expected literal",constant_value);
        }
        return std::make_unique<ast::Constant>(constant_value);
    }
    
    std::unique_ptr<ast::UnaryOp> parse_unary_op(lexer::Lexer& l){
        auto op_token = l.get_token();
        if(!matches_type(op_token,
            token::TokenType::Minus,
            token::TokenType::Plus,
            token::TokenType::BitwiseNot,
            token::TokenType::Not)){
            throw parse_error::ParseError("Not valid unary operator",op_token);
        }
        auto expr = parse_expr(l, unary_op_binding_power);
        return std::make_unique<ast::UnaryOp>(op_token,std::move(expr));
    }

    std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::Lexer& l, std::unique_ptr<ast::Expr> left, int min_bind_power){
        auto op_token = l.get_token();
        if(binary_op_binding_power.find(op_token.type) == binary_op_binding_power.end()){
            throw parse_error::ParseError("Not valid binary operator",op_token);
        }
        auto right = parse_expr(l, min_bind_power);
        return std::make_unique<ast::BinaryOp>(op_token,std::move(left),std::move(right));
    }

    std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power){
        auto expr_start = l.peek_token();
        std::unique_ptr<ast::Expr> expr_ptr = nullptr;
        if(matches_type(expr_start, 
            token::TokenType::IntegerLiteral, 
            token::TokenType::FloatLiteral)){
            expr_ptr =  parse_constant(l);
        }
        if(matches_type(expr_start,
            token::TokenType::Minus,
            token::TokenType::Plus,
            token::TokenType::BitwiseNot,
            token::TokenType::Not)){
            expr_ptr =  parse_unary_op(l);
        }
        if(matches_type(expr_start,token::TokenType::LParen)){
            l.get_token();
            expr_ptr = parse_expr(l);
            check_token_type(l.get_token(), token::TokenType::RParen);
        }
        if(expr_ptr == nullptr){
            throw parse_error::ParseError("Expected beginning of expression",l.peek_token());
        }
        //While the next thing is an operator of high precedence, keep parsing
        while(true){
            auto potential_op_token = l.peek_token();
            if(binary_op_binding_power.find(potential_op_token.type) == binary_op_binding_power.end()){
                break; //Not an operator
            }
            auto binding_power = binary_op_binding_power.at(potential_op_token.type);
            if(binding_power.first < min_bind_power){
                break;
            }
            expr_ptr = parse_binary_op(l, std::move(expr_ptr), binding_power.second);
        }
        return expr_ptr;
    }

    std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l){
        auto return_keyword = l.get_token();
        check_token_type(return_keyword, token::TokenType::Keyword);

        if(!matches_keyword(return_keyword, "return")){
            throw parse_error::ParseError("Expected keyword \"return\"", return_keyword);
        }
        auto ret_value = parse_expr(l);
        auto semicolon = l.get_token();
        check_token_type(semicolon, token::TokenType::Semicolon);
        return std::make_unique<ast::ReturnStmt>(std::move(ret_value));
    }

    std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l){
        auto ret_type = l.get_token();
        if(ret_type.type != token::TokenType::Keyword && ret_type.type != token::TokenType::Identifier){
            throw parse_error::ParseError("Invalid function return type", ret_type);
        }
        auto name = l.get_token();
        if(name.type != token::TokenType::Identifier){
            throw parse_error::ParseError("Invalid function name", name);
        }

        check_token_type(l.get_token(), token::TokenType::LParen);
        check_token_type(l.get_token(), token::TokenType::RParen);
        check_token_type(l.get_token(), token::TokenType::LBrace);

        auto function_body = parse_return_stmt(l);

        check_token_type(l.get_token(), token::TokenType::RBrace);
        return std::make_unique<ast::FunctionDef>(name.value, ret_type.value, std::move(function_body));
    }
}//namespace

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    auto main_method = parse_function_def(l);
    return std::make_unique<ast::Program>(std::move(main_method));
}

} //namespace parse
