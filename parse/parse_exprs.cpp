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

    //Operator Precedence
    constexpr int ternary_cond_binding_power = 6;
    //Unary operators should bind more tightly than any normal binary op
    constexpr int unary_op_binding_power = 40;
    //Higher prescedence than comma, lower than assignment
    constexpr int func_call_arg_binding_power = 3;
    //left binding is less than right binding for left associativity (+, -)
    //and vice versa for right associativity
    std::map<token::TokenType, std::pair<int, int>> binary_op_binding_power = {{
        {token::TokenType::Star, {25,26}}, {token::TokenType::Div, {25,26}},
        {token::TokenType::Mod, {25,26}}, 
        {token::TokenType::Plus,{23,24}}, {token::TokenType::Minus,{23,24}}, 
        {token::TokenType::LShift,{21,22}}, {token::TokenType::RShift,{21,22}}, 
        {token::TokenType::Less, {19,20}}, {token::TokenType::Greater, {19,20}},
        {token::TokenType::LEq, {19,20}}, {token::TokenType::GEq, {19,20}},
        {token::TokenType::Equal, {17,18}}, {token::TokenType::NEqual, {17,18}},
        {token::TokenType::Amp, {15,16}},
        {token::TokenType::BitwiseXor, {13,14}}, {token::TokenType::BitwiseOr, {11,12}},
        {token::TokenType::And, {9,10}}, {token::TokenType::Or, {7,8}},
        {token::TokenType::Assign, {5,4}},
        {token::TokenType::LSAssign, {5,4}},{token::TokenType::RSAssign, {5,4}},
        {token::TokenType::BOAssign, {5,4}},{token::TokenType::BXAssign, {5,4}},
        {token::TokenType::ModAssign, {5,4}},{token::TokenType::BAAssign, {5,4}},
        {token::TokenType::MultAssign, {5,4}},{token::TokenType::DivAssign, {5,4}},
        {token::TokenType::PlusAssign, {5,4}},{token::TokenType::MinusAssign, {5,4}},
        {token::TokenType::Comma, {1,2}},
    }};

std::unique_ptr<ast::InitializerList> parse_initializer_list(lexer::TokenStream& l){
    auto inits = std::vector<std::unique_ptr<ast::Initializer>>{};
    auto tok = l.get_token();
    check_token_type(tok, token::TokenType::LBrace);
    while(l.peek_token().type != token::TokenType::RBrace){
        if(l.peek_token().type == token::TokenType::LBrace){
            inits.push_back(parse_initializer_list(l));
        }else{
            inits.push_back(parse_expr(l,binary_op_binding_power.at(token::TokenType::Assign).second));
        }
        if(token::matches_type(l.peek_token(),token::TokenType::RBrace)){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    check_token_type(l.get_token(), token::TokenType::RBrace);
    return std::make_unique<ast::InitializerList>(tok, std::move(inits));
}


std::unique_ptr<ast::StrLiteral> parse_str_literal(lexer::TokenStream& l){
    auto literals = std::vector<token::Token>{};
    if(l.peek_token().type != token::TokenType::StrLiteral){
        throw parse_error::ParseError("Expected string",l.peek_token());
    }
    do{
        literals.push_back(l.get_token());
    }while(l.peek_token().type == token::TokenType::StrLiteral);
    return std::make_unique<ast::StrLiteral>(literals);
}
std::unique_ptr<ast::Constant> parse_constant(lexer::TokenStream& l){
    auto constant_value = l.get_token();
    if(!token::matches_type(constant_value, 
                token::TokenType::IntegerLiteral, 
                token::TokenType::FloatLiteral)){
        throw parse_error::ParseError("Expected literal",constant_value);
    }
    return std::make_unique<ast::Constant>(constant_value);
}
    
std::unique_ptr<ast::UnaryOp> parse_unary_op(lexer::TokenStream& l){
    auto op_token = l.get_token();
    if(!token::matches_type(op_token,
                token::TokenType::Minus,
                token::TokenType::Plus,
                token::TokenType::BitwiseNot,
                token::TokenType::Amp,
                token::TokenType::Star,
                token::TokenType::Not,
                token::TokenType::Plusplus,//Prefix versions
                token::TokenType::Minusminus)){
        throw parse_error::ParseError("Not valid unary operator",op_token);
    }
    auto expr = parse_expr(l, unary_op_binding_power);
    return std::make_unique<ast::UnaryOp>(op_token,std::move(expr));
}


std::unique_ptr<ast::Conditional> parse_conditional(lexer::TokenStream& l, std::unique_ptr<ast::Expr> cond){
    auto question = l.get_token();
    check_token_type(question, token::TokenType::Question);
    auto true_expr = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto false_expr = parse_expr(l,ternary_cond_binding_power);
    return std::make_unique<ast::Conditional>(question, std::move(cond),std::move(true_expr),std::move(false_expr));
}

std::unique_ptr<ast::Alignof> parse_alignof(lexer::TokenStream& l){
    auto tok = l.get_token();
    assert(tok.value == "_Alignof");
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto arg = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    return std::make_unique<ast::Alignof>(tok, std::move(arg));
}
std::unique_ptr<ast::Sizeof> parse_sizeof(lexer::TokenStream& l){
    auto tok = l.get_token();
    assert(tok.value == "sizeof");
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto arg = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    return std::make_unique<ast::Sizeof>(tok, std::move(arg));
}
std::unique_ptr<ast::FuncCall> parse_function_call(lexer::TokenStream& l, std::unique_ptr<ast::Expr> func){
    auto tok = l.get_token();
    check_token_type(tok, token::TokenType::LParen);
    auto args = std::vector<std::unique_ptr<ast::Expr>>{};
    while(l.peek_token().type != token::TokenType::RParen){
        args.push_back(parse_expr(l,func_call_arg_binding_power));
        if(l.peek_token().type == token::TokenType::RParen){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    return std::make_unique<ast::FuncCall>(tok, std::move(func), std::move(args));
}

std::unique_ptr<ast::MemberAccess> parse_member_access(lexer::TokenStream& l, std::unique_ptr<ast::Expr> arg){
    auto op_token = l.get_token();
    check_token_type(op_token, token::TokenType::Period);
    auto index = l.get_token();
    check_token_type(index, token::TokenType::Identifier);
    return std::make_unique<ast::MemberAccess>(op_token,std::move(arg), index.value);
}
std::unique_ptr<ast::ArrayAccess> parse_array_access(lexer::TokenStream& l, std::unique_ptr<ast::Expr> arg){
    auto op_token = l.get_token();
    check_token_type(op_token, token::TokenType::LBrack);
    auto index = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RBrack);
    return std::make_unique<ast::ArrayAccess>(op_token,std::move(arg), std::move(index));
}
std::unique_ptr<ast::Postfix> parse_postfix(lexer::TokenStream& l, std::unique_ptr<ast::Expr> arg){
    auto op_token = l.get_token();
    if(!token::matches_type(op_token,
                token::TokenType::Plusplus,
                token::TokenType::Minusminus)){
        throw parse_error::ParseError("Not valid postfix operator",op_token);
    }
    return std::make_unique<ast::Postfix>(op_token,std::move(arg));
}
} //anon namespace

std::unique_ptr<ast::Decl> parse_init_decl(lexer::TokenStream& l, Declarator declarator){
    auto var_name = declarator.first.value();
    check_token_type(var_name, token::TokenType::Identifier);
    if(l.peek_token().type == token::TokenType::Assign){
        if(type::is_type<type::VoidType>(declarator.second)){
            throw sem_error::TypeError("Invalid type 'void'", var_name);
        }
        if(type::is_type<type::FuncType>(declarator.second)){
            throw sem_error::TypeError("Invalid assignment to function type", var_name);
        }
        l.get_token();
        if(l.peek_token().type == token::TokenType::LBrace){
            auto assign = parse_initializer_list(l);
            return std::make_unique<ast::VarDecl>(var_name, declarator.second, std::move(assign));
        }else{
            auto assign = parse_expr(l,binary_op_binding_power.at(token::TokenType::Assign).second);
            return std::make_unique<ast::VarDecl>(var_name, declarator.second, std::move(assign));
        }
    }else{
        if(type::is_type<type::VoidType>(declarator.second)){
            throw sem_error::TypeError("Invalid type 'void'", var_name);
        }
        if(type::is_type<type::FuncType>(declarator.second)){
            return std::make_unique<ast::FunctionDecl>(var_name, type::get<type::FuncType>(declarator.second));
        }else{
            return std::make_unique<ast::VarDecl>(var_name, declarator.second);
        }
    }
}
std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::TokenStream& l, std::unique_ptr<ast::Expr> left, int min_bind_power){
    auto op_token = l.get_token();
    if(binary_op_binding_power.find(op_token.type) == binary_op_binding_power.end()){
        throw parse_error::ParseError("Not valid binary operator",op_token);
    }
    auto right = parse_expr(l, min_bind_power);
    return std::make_unique<ast::BinaryOp>(op_token,std::move(left),std::move(right));
}
std::unique_ptr<ast::Variable> parse_variable(lexer::TokenStream& l){
    auto var_tok = l.get_token();
    check_token_type(var_tok, token::TokenType::Identifier);
    return std::make_unique<ast::Variable>(var_tok);
}
std::unique_ptr<ast::Expr> parse_expr(lexer::TokenStream& l, int min_bind_power){
    auto expr_start = l.peek_token();
    std::unique_ptr<ast::Expr> expr_ptr = nullptr;
    switch(expr_start.type){
        case token::TokenType::IntegerLiteral:
        case token::TokenType::FloatLiteral:
            expr_ptr =  parse_constant(l);
            break;
        case token::TokenType::Minus:
        case token::TokenType::Plus:
        case token::TokenType::BitwiseNot:
        case token::TokenType::Amp:
        case token::TokenType::Star:
        case token::TokenType::Not:
        case token::TokenType::Plusplus:
        case token::TokenType::Minusminus:
            expr_ptr =  parse_unary_op(l);
            break;
        case token::TokenType::Identifier:
            expr_ptr = parse_variable(l);
            break;
        case token::TokenType::LParen:
            l.get_token();
            expr_ptr = parse_expr(l);
            check_token_type(l.get_token(), token::TokenType::RParen);
            break;
        case token::TokenType::StrLiteral:
            expr_ptr =  parse_str_literal(l);
            break;
        case token::TokenType::Keyword:
            if(expr_start.value == "sizeof"){
                expr_ptr =  parse_sizeof(l);
                break;
            }
            if(expr_start.value == "_Alignof"){
                expr_ptr =  parse_alignof(l);
                break;
            }
            throw parse_error::ParseError("Unknown keyword starting expression",expr_start);
            break;
    }
    if(expr_ptr == nullptr){
        throw parse_error::ParseError("Expected beginning of expression",l.peek_token());
    }
    //While the next thing is an operator of high precedence, keep parsing
    while(true){
        auto potential_op_token = l.peek_token();
        if(potential_op_token.type == token::TokenType::Question){//Ternary conditional
            if(ternary_cond_binding_power < min_bind_power){
                break;
            }
            expr_ptr = parse_conditional(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::LParen){
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_function_call(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::LBrack){
            //postfix array access
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_array_access(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::Period){
            //postfix struct access
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_member_access(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::Plusplus ||
            potential_op_token.type == token::TokenType::Minusminus){
            //postfix increment/decrement
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_postfix(l, std::move(expr_ptr));
            continue;
        }
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

} //namespace parse
