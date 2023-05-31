#include "type.h"
#include "parse.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
#include <map>
#include <set>
namespace parse{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{
    //Check and throw default unexpected token exception
    void check_token_type(const token::Token& tok, token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + token::string_name(type), tok);
        }
    }

    type::CType parse_pointer(type::CType specified_type, lexer::Lexer& l){
        if(l.peek_token().type != token::TokenType::Mult){
            return specified_type;
        }else{
            throw parse_error::UnknownError("Pointer declarations not yet implemented", l.peek_token());
        }
    }
    std::pair<Declarator,std::optional<std::vector<Declarator>>> parse_declarator(type::CType specified_type, lexer::Lexer& l){
        type::CType type = parse_pointer(specified_type, l);
        auto next_tok = l.peek_token();
        if(next_tok.type != token::TokenType::Identifier){
            if(next_tok.type == token::TokenType::LParen){
                l.get_token();
                auto ret_val = parse_declarator(type,l);
                check_token_type(l.get_token(), token::TokenType::RParen);
                return ret_val;
            }else{
                throw parse_error::ParseError("Expected identifier or left parenthesis", next_tok);
            }
        }
        auto ident = l.get_token();
        check_token_type(ident, token::TokenType::Identifier);
        next_tok = l.peek_token();
        switch(next_tok.type){
            case token::TokenType::LParen:
                {
                    auto param_list = parse_param_list(type, l);
                    type = param_list.first;
                    return std::make_pair(std::make_pair(ident, type),param_list.second);
                }
            case token::TokenType::LBrack:
                throw parse_error::ParseError("Expected identifier or left parenthesis", next_tok);
            default:
                return std::make_pair(std::make_pair(ident, type),std::nullopt);
        }
        //Above switch should definitely return
        __builtin_unreachable();
    }
} //namespace

std::pair<type::FuncType, std::vector<Declarator>> parse_param_list(type::CType ret_type, lexer::Lexer& l){
    check_token_type(l.get_token(), token::TokenType::LParen);
    if(l.peek_token().type == token::TokenType::Ellipsis){
        throw parse_error::ParseError("Named parameter required before \"...\"", l.peek_token());
    }
    bool variadic = false;
    auto declarators = std::vector<Declarator>{};
    auto params = std::vector<type::CType>{};
    auto names = std::set<std::string>{};
    while(true){
        if(l.peek_token().type == token::TokenType::RParen){ //K & R syntax with empty param list
            break;
        }
        type::CType param_specifiers = parse_specifiers(l);
        if(token::matches_type(l.peek_token(),token::TokenType::Comma,token::TokenType::RParen)){
            declarators.push_back(std::make_pair(std::nullopt, param_specifiers));
            params.push_back(param_specifiers);
        }else{
            declarators.push_back(parse_declarator(param_specifiers,l).first);
            params.push_back(declarators.back().second);
            if(params.back() == type::CType(type::VoidType())){
                throw sem_error::TypeError("Cannot have variable of void type",declarators.back().first.value());
            }
            if(declarators.back().first.has_value()){
                if(names.insert(declarators.back().first.value().value).second == false){
                    throw sem_error::STError("Duplicate variable name in function parameter list",declarators.back().first.value());
                }
            }
        }
        if(token::matches_type(l.peek_token(),token::TokenType::Comma)){
            l.get_token();
        }else{
            break;
        }
    }
    if(token::matches_type(l.peek_token(),token::TokenType::Ellipsis)){
        l.get_token();
        variadic = true;
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    try{
        return std::make_pair(type::FuncType(ret_type, params, variadic),declarators);
    }catch(std::runtime_error& e){
        throw sem_error::TypeError(std::string("Failure to construct Function Type ") +e.what(),l.peek_token());
    }
}



std::unique_ptr<ast::DeclList> parse_decl_list(lexer::Lexer& l){
    auto specifiers = parse_specifiers(l);
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    while(true){
        auto declarator = parse_declarator(specifiers, l).first;
        if(!declarator.first.has_value()){
            throw parse_error::ParseError("Abstract declarator not permitted here", l.peek_token());
        }
        decls.push_back(parse_init_decl(l, specifiers, declarator));
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    return std::make_unique<ast::DeclList>(std::move(decls));
}
std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l, Declarator decl, std::vector<Declarator> params){
    check_token_type(l.peek_token(), token::TokenType::LBrace);
    auto param_decls = std::vector<std::unique_ptr<ast::Decl>>{};
    for(const auto& param_declarator: params){
        auto param_decl = std::visit(type::make_visitor<std::unique_ptr<ast::Decl>>(
            [&](const type::BasicType& bt){
                if(param_declarator.first.has_value()){
                    return std::make_unique<ast::VarDecl>(param_declarator.first.value(),bt);
                }else{
                    throw sem_error::STError("Cannot have missing parameter name in function def", l.peek_token());
                }
            },
            [&](const type::VoidType& vt){
                if(param_declarator.first.has_value()){
                    throw sem_error::TypeError("Cannot have void type with parameter name", param_declarator.first.value());
                }
                if(params.size() >1){
                    throw sem_error::TypeError("Cannot have void type in function with multiple parameters",l.peek_token());
                }
                return nullptr;
            },
            [&l](const type::FuncType& ft){throw sem_error::TypeError("Cannot have function as parameter type", l.peek_token());}
        ),param_declarator.second);
        if(param_decl != nullptr){
            param_decls.push_back(std::move(param_decl));
        }
    }
    auto function_body = parse_compound_stmt(l);
    return std::make_unique<ast::FunctionDef>(decl.first.value(), std::get<type::DerivedType>(decl.second).get<type::FuncType>(), 
        std::move(param_decls), std::move(function_body));
}

std::unique_ptr<ast::ExtDecl> parse_ext_decl(lexer::Lexer& l){
    while(l.peek_token().type == token::TokenType::Semicolon){
        l.get_token();
    }
    auto specifiers = parse_specifiers(l);
    auto declarator_param_pair = parse_declarator(specifiers, l);
    if(l.peek_token().type == token::TokenType::LBrace){
        if(!type::is_type<type::FuncType>(declarator_param_pair.first.second)){
            throw parse_error::ParseError("Expected function definition", l.peek_token());
        }
        return parse_function_def(l, declarator_param_pair.first, declarator_param_pair.second.value());
    }
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    decls.push_back(parse_init_decl(l,specifiers, declarator_param_pair.first));
    while(true){
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            l.get_token();
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
        auto declarator = parse_declarator(specifiers, l).first;
        if(!declarator.first.has_value()){
            throw parse_error::ParseError("Abstract declarator not permitted here", l.peek_token());
        }
        decls.push_back(parse_init_decl(l, specifiers, declarator));
    }
    return std::make_unique<ast::DeclList>(std::move(decls));
}
} //namespace parse
