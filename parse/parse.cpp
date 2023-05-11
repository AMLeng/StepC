#include "parse.h"
#include "type.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
#include <map>
namespace parse{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{

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
}//namespace

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
std::unique_ptr<ast::Variable> parse_variable(lexer::Lexer& l){
    auto var_tok = l.get_token();
    check_token_type(var_tok, token::TokenType::Identifier);
    return std::make_unique<ast::Variable>(var_tok);
}
std::unique_ptr<ast::LValue> parse_lvalue(lexer::Lexer& l){
    //Right now variables are the only implemented lvalue
    return parse_variable(l);
}
std::unique_ptr<ast::Assign> parse_assign(std::unique_ptr<ast::LValue> var, lexer::Lexer& l){
    auto equal_sign = l.get_token();
    check_token_type(equal_sign, token::TokenType::Assign);
    auto right = parse_expr(l);
    return std::make_unique<ast::Assign>(equal_sign, std::move(var), std::move(right));
}

std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power){
    auto expr_start = l.peek_token();
    std::unique_ptr<ast::Expr> expr_ptr = nullptr;
    if(matches_type(expr_start, 
                token::TokenType::IntegerLiteral, 
                token::TokenType::FloatLiteral)){
        assert(expr_ptr == nullptr && "Already parsed expression");
        expr_ptr =  parse_constant(l);
    }
    if(matches_type(expr_start,
                token::TokenType::Minus,
                token::TokenType::Plus,
                token::TokenType::BitwiseNot,
                token::TokenType::Not)){
        assert(expr_ptr == nullptr && "Already parsed expression");
        expr_ptr =  parse_unary_op(l);
    }
    if(matches_type(expr_start,token::TokenType::Identifier)){
        auto lvalue = parse_lvalue(l);
        auto maybe_assign = l.peek_token();
        if(matches_type(maybe_assign,token::TokenType::Assign)){
            expr_ptr = parse_assign(std::move(lvalue), l);
        }else{
            expr_ptr = std::move(lvalue);
        }
    }
    if(matches_type(expr_start,token::TokenType::LParen)){
        assert(expr_ptr == nullptr && "Already parsed expression");
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
std::unique_ptr<ast::VarDecl> parse_var_decl(lexer::Lexer& l){
    auto keyword_list = std::multiset<std::string>{};
    auto first_keyword = l.peek_token();
    while(l.peek_token().type == token::TokenType::Keyword){
        keyword_list.insert(l.get_token().value);
    }
    if(keyword_list.size() == 0){
        throw parse_error::ParseError("Parsing decl that did not start with a keyword", first_keyword);
    }
    type::BasicType t;
    try{
        t = type::from_str_multiset(keyword_list);
    }catch(std::runtime_error& e){
        throw sem_error::TypeError(e.what(), first_keyword);
    }
    auto var_name = l.get_token();
    check_token_type(var_name, token::TokenType::Identifier);
    auto next_tok = l.peek_token();
    if(matches_type(next_tok, token::TokenType::Semicolon)){
        check_token_type(l.get_token(), token::TokenType::Semicolon);
        return std::make_unique<ast::VarDecl>(var_name, t);
    }
    check_token_type(next_tok, token::TokenType::Assign);
    std::unique_ptr<ast::LValue> var = std::make_unique<ast::Variable>(var_name);
    auto assign = parse_assign(std::move(var),l);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::VarDecl>(var_name, t, std::move(assign));
}

std::unique_ptr<ast::Stmt> parse_stmt(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && matches_keyword(next_token, "return")){
        return parse_return_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword){
        return parse_var_decl(l);
    }
    //If is a typedef name will also parse var decl, but that's for later
    auto expr = parse_expr(l);
    auto semicolon = l.get_token();
    check_token_type(semicolon, token::TokenType::Semicolon);
    return std::move(expr);
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

    auto function_body = std::vector<std::unique_ptr<ast::Stmt>>{};
    while(l.peek_token().type != token::TokenType::RBrace){
        function_body.push_back(parse_stmt(l));
    }

    check_token_type(l.get_token(), token::TokenType::RBrace);
    return std::make_unique<ast::FunctionDef>(name.value, type::from_str(ret_type.value), std::move(function_body));
}

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    auto main_method = parse_function_def(l);
    return std::make_unique<ast::Program>(std::move(main_method));
}

} //namespace parse
