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
        std::optional<std::vector<Declarator>> param_list;
        int index;
        public:
        TypeBuilder() : index(0), ident(std::nullopt){
            unapplied.emplace_back();
        }
        std::optional<std::vector<Declarator>> get_params(){
            return param_list;
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
        void add_array(token::Token tok, std::vector<std::optional<int>> sizes){ 
            if(unapplied.at(index).size() > 0 ){
                auto prev_type = unapplied.at(index).back().second.type;
                if(prev_type == token::TokenType::LParen){
                    throw parse_error::ParseError("Array element cannot have function type",tok);
                }
            }
            unapplied.at(index).push_back(
                std::make_pair([=](type::CType t){
                    for(int i=sizes.size()-1; i>=0; i--){
                        t =  type::ArrayType(t, sizes.at(i));
                    }
                    return t;
                },tok)
            );
        }
        void add_func(std::pair<std::vector<Declarator>,bool>&& parsed_param_list, token::Token tok){ 
            if(index == unapplied.size()-1 && ident.has_value()){
                //If this is the parameter list immediately following the identifier at the deepest level we've seen
                assert(param_list == std::nullopt);
                param_list = parsed_param_list.first;
            }
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
            auto storage_specifiers = current.storage;
            while(unapplied.size() > 0){
                while(unapplied.front().size() > 0){
                    if(storage_specifiers.has_value() && storage_specifiers.value() == type::SSpecifier::Typedef){
                        current.storage = std::nullopt;
                    }
                    try{
                        current = unapplied.front().front().first(current);
                    }catch(std::exception& e){
                        throw parse_error::ParseError(e.what(), unapplied.front().front().second);
                    }
                    current.storage = storage_specifiers;
                    unapplied.front().pop_front();
                }
                unapplied.pop_front();
            }
            return std::make_pair(ident, current);
        }
    };

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
                {
                auto lbrack = l.peek_token();
                auto sizes = std::vector<std::optional<int>>{};
                while(l.peek_token().type == token::TokenType::LBrack){
                    l.get_token();
                    std::optional<int> size = std::nullopt;
                    if(l.peek_token().type != token::TokenType::RBrack){
                        auto expr = parse_expr(l);
                        auto temp_st = symbol::GlobalTable();
                        expr->analyze(&temp_st);
                        if(!std::holds_alternative<long long int>(expr->constant_value)){
                            throw sem_error::TypeError("Invalid constant integer expr for array size", expr->tok);
                        }
                        size = std::get<long long int>(expr->constant_value);
                    }
                    sizes.push_back(size);
                    check_token_type(l.get_token(),token::TokenType::RBrack);
                }
                for(int i=1; i<sizes.size(); i++){
                    if(!sizes.at(i).has_value()){
                        throw sem_error::TypeError("Cannot have inner nested array of indeterminate size", l.peek_token());
                    }
                }
                builder.add_array(lbrack, sizes);
                parse_declarator_helper(l,builder);
                return;
                }
            default:
                return;
        }
    }

void handle_abstract_decl(Declarator declarator, token::Token tok){
    if(type::is_type<type::StructType>(declarator.second)){
        //Do nothing, since handled separately as a TagDecl in parse_specifiers
        return;
    }
    if(type::is_type<type::UnionType>(declarator.second)){
        //Do nothing, since handled separately as a TagDecl in parse_specifiers
        return;
    }
    throw parse_error::ParseError("Abstract declarator not permitted here", tok);
}
} //namespace
Declarator parse_declarator(type::CType type, lexer::Lexer& l){
    //This does not parse declarators for function definitions
    auto builder = TypeBuilder();
    parse_declarator_helper(l,builder);
    auto ret =  builder.build_declarator(type);
    return ret;
}

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
        auto param_specifiers = parse_specifiers(l);
        if(param_specifiers.second.size() > 0){
            throw sem_error::TypeError("Struct/Union definition or typedef in function parameter list is useless", l.peek_token());
        }
        declarators.push_back(parse_declarator(param_specifiers.first,l));
        if(type::is_type<type::VoidType>(declarators.back().second)){
            if(declarators.back().first.has_value()){
                throw sem_error::TypeError("Cannot have named variable of void type",declarators.back().first.value());
            }
            if(declarators.size() > 1 ){
                throw sem_error::TypeError("Cannot have void type in function with multiple parameters",l.peek_token());
            }
        }
        if(type::is_type<type::FuncType>(declarators.back().second)){
            declarators.back().second = type::PointerType(declarators.back().second);
        }
        if(type::is_type<type::ArrayType>(declarators.back().second)){
            //Splice array type to just be a pointer
            declarators.back().second = type::get<type::PointerType>(declarators.back().second);
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
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    auto specifiers = parse_specifiers(l);
    auto type_decls = std::move(specifiers.second);
    while(true){
        auto declarator = parse_declarator(specifiers.first, l);
        if(!declarator.first.has_value()){
            handle_abstract_decl(declarator, l.peek_token());
        }else{
            if(declarator.second.storage == std::optional<type::SSpecifier>(type::SSpecifier::Typedef)){
                type_decls.push_back(std::make_unique<ast::TypedefDecl>(declarator.first.value(), declarator.second));
            }else{
                decls.push_back(parse_init_decl(l, declarator));
            }
        }
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    return std::make_unique<ast::DeclList>(std::move(decls), std::move(type_decls));
}

std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l, std::vector<Declarator> params, 
    Declarator func, std::vector<std::unique_ptr<ast::TypeDecl>> tags){
    auto param_decls = std::vector<std::unique_ptr<ast::VarDecl>>{};
    for(const auto& param_declarator: params){
        if(!param_declarator.first.has_value()){
            if(params.size() > 1 || !type::is_type<type::VoidType>(param_declarator.second)){
                throw parse_error::ParseError("Function parameter missing identifier", l.peek_token());
            }
            break;
        }
        param_decls.push_back(std::make_unique<ast::VarDecl>(param_declarator.first.value(),param_declarator.second));
    }
    auto function_body = parse_compound_stmt(l);
    return std::make_unique<ast::FunctionDef>(func.first.value(), type::get<type::FuncType>(func.second), 
        std::move(param_decls), std::move(function_body), std::move(tags));
}

std::unique_ptr<ast::ExtDecl> parse_ext_decl(lexer::Lexer& l){
    while(l.peek_token().type == token::TokenType::Semicolon){
        l.get_token();
    }
    if(!type::is_specifier(l.peek_token().value) && !(l.peek_token().type == token::TokenType::Identifier)){
        throw parse_error::ParseError("Invalid start to external declaration", l.peek_token());
    }
    auto specifiers = parse_specifiers(l);
    auto specified_type = specifiers.first;
    auto type_decls = std::move(specifiers.second);
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    {
        //We can't just call parse_declarator since here we need
        //Access to the names in the parameter list parsed by the TypeBuilder
        auto builder = TypeBuilder();
        parse_declarator_helper(l,builder);
        auto declarator = builder.build_declarator(specified_type);
        if(l.peek_token().type ==token::TokenType::LBrace){
            auto params = builder.get_params();
            if(!params.has_value()){
                throw parse_error::ParseError("Unexpected beginning of function definition", l.peek_token());
            }
            return parse_function_def(l, params.value(), declarator, std::move(type_decls));
        }
        if(!declarator.first.has_value()){
            handle_abstract_decl(declarator, l.peek_token());
        }else{
            if(declarator.second.storage == std::optional<type::SSpecifier>(type::SSpecifier::Typedef)){
                type_decls.push_back(std::make_unique<ast::TypedefDecl>(declarator.first.value(), declarator.second));
            }else{
                decls.push_back(parse_init_decl(l, declarator));
            }
        }
    }

    while(true){
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            l.get_token();
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
        auto declarator = parse_declarator(specified_type, l);
        if(!declarator.first.has_value()){
            handle_abstract_decl(declarator, l.peek_token());
        }else{
            if(declarator.second.storage == std::optional<type::SSpecifier>(type::SSpecifier::Typedef)){
                type_decls.push_back(std::make_unique<ast::TypedefDecl>(declarator.first.value(), declarator.second));
            }else{
                decls.push_back(parse_init_decl(l, declarator));
            }
        }
    }
    return std::make_unique<ast::DeclList>(std::move(decls), std::move(type_decls));
}
} //namespace parse
