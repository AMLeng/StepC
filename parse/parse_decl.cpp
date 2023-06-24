#include "type.h"
#include "parse.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
#include <map>
#include <set>
#include <deque>
#include <functional>
#include <tuple>
namespace parse{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{
    type::FuncType compute_function_type(type::CType ret_type, std::pair<std::vector<Declarator>,bool> parsed_params){
        if(parsed_params.first.size() > 0){
            auto params = std::vector<type::CType>{};
            for(const auto& decl : parsed_params.first){
                params.emplace_back(decl.second);
            }
            return type::FuncType(ret_type, params, parsed_params.second);
        }else{
            return type::FuncType(ret_type);
        }
    }
    class TypeBuilder{
        std::deque<std::deque<std::pair<std::function<type::CType(type::CType)>,token::Token>>> unapplied;
        std::optional<token::Token> ident;
        int index;
        public:
        TypeBuilder() : index(0), ident(std::nullopt){
            unapplied.emplace_back();
        }
        void add_ident(token::Token tok){
            if(index != unapplied.size()-1){
                throw parse_error::ParseError("Invalid type definition ",tok);
            }
            if(ident.has_value()){
                throw parse_error::ParseError("Multiple identifiers in type definition ",tok);
            }
            ident = tok;
        }
        void add_level(token::Token tok){
            if(index != unapplied.size()-1){
                throw parse_error::ParseError("Invalid type definition ",tok);
            }
            if(ident.has_value()){
                throw parse_error::ParseError("Invalid type definition ",tok);
            }
            unapplied.emplace_back();
            index++;
        }
        void subtract_level(token::Token tok){
            index--;
            if(index < 0){
                throw parse_error::ParseError("Invalid type definition ",tok);
            }
        }
        void add_pointer(token::Token tok){
            if(ident.has_value()){
                throw parse_error::ParseError("Cannot have pointers after ident in type declarator ",tok);
            }
            if(unapplied.at(index).size() > 0 
                && unapplied.at(index).back().second.type != token::TokenType::Star){
                throw parse_error::ParseError("Cannot have pointers after other operations in type declarator ",tok);
            }
            unapplied.at(index).push_back(
                std::make_pair([](type::CType t){return type::PointerType(t);},tok)
            );
        }
        void add_func(std::pair<std::vector<Declarator>,bool>&& parsed_param_list, token::Token tok){ 
            if(unapplied.at(index).size() > 0 ){
                auto prev_type = unapplied.at(index).back().second.type;
                if(prev_type == token::TokenType::LParen){
                    throw parse_error::ParseError("Function declarator cannot have function return type",tok);
                }
                if(prev_type == token::TokenType::LBrack){
                    throw parse_error::ParseError("Function declarator cannot have array return type",tok);
                }
            }
            unapplied.at(index).push_back(
                std::make_pair([params = std::move(parsed_param_list)](type::CType t){
                    return compute_function_type(t,params);
                },tok)
            );
        }
        Declarator build_declarator(type::CType current){
            while(unapplied.size() > 0){
                while(unapplied.front().size() > 0){
                    try{
                        current = unapplied.front().front().first(current);
                    }catch(std::exception& e){
                        throw parse_error::ParseError(e.what(), unapplied.front().front().second);
                    }
                    unapplied.front().pop_front();
                }
                unapplied.pop_front();
            }
            return std::make_pair(ident, current);
        }
    };

    //Check and throw default unexpected token exception
    void check_token_type(const token::Token& tok, token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + token::string_name(type), tok);
        }
    }
    void parse_declarator_helper(lexer::Lexer& l, TypeBuilder& builder){
        auto next_tok = l.peek_token();
        switch(next_tok.type){
            case token::TokenType::Identifier:
                builder.add_ident(l.get_token());
                parse_declarator_helper(l,builder);
                return;
            case token::TokenType::Star:
                builder.add_pointer(l.get_token());
                parse_declarator_helper(l,builder);
                return;
            case token::TokenType::LParen:
                if(l.peek_token(2).type == token::TokenType::Keyword ||
                    l.peek_token(2).type == token::TokenType::RParen){
                    //Function declaration
                    auto tok = l.peek_token();
                    auto parsed_param_pair = parse_param_list(l);
                    builder.add_func(std::move(parsed_param_pair),tok);
                }else{
                    //Parens for binding
                    builder.add_level(l.get_token());
                    parse_declarator_helper(l,builder);
                    check_token_type(l.peek_token(),token::TokenType::RParen);
                    builder.subtract_level(l.get_token());
                }
                parse_declarator_helper(l,builder);
                return;
            case token::TokenType::LBrack:
                throw parse_error::ParseError("Unexpected token when parsing declarator", next_tok);
            default:
                return;
        }
    }

    Declarator parse_declarator(type::CType type, lexer::Lexer& l){
        //This does not parse declarators for function definitions
        auto builder = TypeBuilder();
        parse_declarator_helper(l,builder);
        return builder.build_declarator(type);
    }
} //namespace

std::pair<std::vector<Declarator>,bool> parse_param_list(lexer::Lexer& l){
    check_token_type(l.get_token(), token::TokenType::LParen);
    if(l.peek_token().type == token::TokenType::Ellipsis){
        throw parse_error::ParseError("Named parameter required before \"...\"", l.peek_token());
    }
    bool variadic = false;
    auto declarators = std::vector<Declarator>{};
    auto names = std::set<std::string>{};
    while(true){
        if(l.peek_token().type == token::TokenType::RParen){
            break;
        }
        if(l.peek_token().type == token::TokenType::Ellipsis){
            variadic = true;
            l.get_token();
            break;
        }
        type::CType param_specifiers = parse_specifiers(l);
        declarators.push_back(parse_declarator(param_specifiers,l));
        if(declarators.back().second == type::CType(type::VoidType())){
            if(declarators.back().first.has_value()){
                throw sem_error::TypeError("Cannot have named variable of void type",declarators.back().first.value());
            }
            if(declarators.size() > 1 ){
                throw sem_error::TypeError("Cannot have void type in function with multiple parameters",l.peek_token());
            }
        }
        if(declarators.back().first.has_value() &&names.insert(declarators.back().first.value().value).second == false){
            throw sem_error::STError("Duplicate variable name in function parameter list",declarators.back().first.value());
        }
        if(token::matches_type(l.peek_token(),token::TokenType::Comma)){
            l.get_token();
        }else{
            break;
        }
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    return std::make_pair(declarators,variadic);
}

std::unique_ptr<ast::DeclList> parse_decl_list(lexer::Lexer& l){
    auto specifiers = parse_specifiers(l);
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    while(true){
        auto declarator = parse_declarator(specifiers, l);
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

std::unique_ptr<ast::FunctionDecl> parse_function_def(lexer::Lexer& l, type::CType ret_type){
    auto ident = l.get_token();
    check_token_type(ident, token::TokenType::Identifier);
    auto parsed_param_pair = parse_param_list(l);
    auto param_declarators = parsed_param_pair.first;

    auto param_decls = std::vector<std::unique_ptr<ast::Decl>>{};
    for(const auto& param_declarator: param_declarators){
        if(param_declarator.first.has_value()){
            auto param_decl = std::make_unique<ast::VarDecl>(param_declarator.first.value(),param_declarator.second);

            assert(param_decl != nullptr && "Failed to create function parameter");
            param_decls.push_back(std::move(param_decl));
        }
    }
    if(l.peek_token().type ==token::TokenType::LBrace){
        //Def
        if(param_decls.size() != param_declarators.size()
            && !(param_declarators.size() == 1 && type::is_type<type::VoidType>(param_declarators.back().second))){
            //If they are not equal, we failed to construct VarDecls for some of the parameters
            //Which only happens if we weren't given identifiers, or if we have no arguments
            throw sem_error::STError("Cannot have missing parameter name in function def", ident);
        }
        auto function_body = parse_compound_stmt(l);
        try{
            auto type = compute_function_type(ret_type, parsed_param_pair);
            return std::make_unique<ast::FunctionDef>(ident, type, std::move(param_decls), std::move(function_body));
        }catch(std::exception& e){
            throw parse_error::ParseError(e.what(), ident);
        }
    }else{
        //Decl
        //We don't move from param_decls so it gets RAIIed
        try{
            auto type = compute_function_type(ret_type, parsed_param_pair);
            return std::make_unique<ast::FunctionDecl>(ident, type);
        }catch(std::exception& e){
            throw parse_error::ParseError(e.what(), ident);
        }
    }
}

std::unique_ptr<ast::ExtDecl> parse_ext_decl(lexer::Lexer& l){
    while(l.peek_token().type == token::TokenType::Semicolon){
        l.get_token();
    }
    auto specified_type = parse_specifiers(l);
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    if(l.peek_token().type == token::TokenType::Identifier
        && l.peek_token(2).type == token::TokenType::LParen){
        auto function_decl_or_def = parse_function_def(l, specified_type);
        if(dynamic_cast<ast::FunctionDef*>(function_decl_or_def.get())){
            return std::unique_ptr<ast::FunctionDef>(dynamic_cast<ast::FunctionDef*>(function_decl_or_def.release()));
        }else{
            decls.push_back(std::move(function_decl_or_def));
        }
    }else{
        decls.push_back(parse_init_decl(l,specified_type, parse_declarator(specified_type, l)));
    }
    while(true){
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            l.get_token();
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
        auto declarator = parse_declarator(specified_type, l);
        if(!declarator.first.has_value()){
            throw parse_error::ParseError("Abstract declarator not permitted here", l.peek_token());
        }
        decls.push_back(parse_init_decl(l, specified_type, declarator));
    }
    return std::make_unique<ast::DeclList>(std::move(decls));
}
} //namespace parse
