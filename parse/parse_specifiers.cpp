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
    type::CType convert_type_specifiers(std::multiset<std::string> specifiers){
        if(specifiers.size() == 0){
            throw std::runtime_error("Failed to parse declaration specifiers");
        }
        type::CType t;
        if(specifiers.size() == 1 && specifiers.find("void") != specifiers.end()){
            return t;
        }
        try{
            return type::from_str_multiset(specifiers);
        }catch(std::runtime_error& e){
            throw;
        }
    }
    std::pair<type::CType,std::vector<std::unique_ptr<ast::TypeDecl>>> parse_tag_specifiers(lexer::Lexer& l, token::Token tag_type){
        std::string ident = "";
        if(l.peek_token().type == token::TokenType::Identifier){
            ident = l.get_token().value;
        }else{
            //Anonymous struct definition, replace the name with a unique anonymous name
            check_token_type(l.peek_token(), token::TokenType::LBrace);
            ident = "anon."+std::to_string(tag_type.loc.start_line)+"."+std::to_string(tag_type.loc.start_col);
        }
        if(l.peek_token().type == token::TokenType::LBrace){
            auto tags = std::vector<std::unique_ptr<ast::TypeDecl>>{};
            l.get_token();
            auto members = std::vector<type::CType>{};
            auto indices = std::map<std::string, int>{};
            while(l.peek_token().type ==token::TokenType::Keyword){
                auto specified = parse_specifiers(l);
                for(auto& t : specified.second){
                    tags.push_back(std::move(t));
                }
                auto declarator = parse_declarator(specified.first, l);
                if(declarator.first.has_value()){
                    indices.emplace(declarator.first.value().value,members.size());
                }
                members.push_back(declarator.second);
                while(l.peek_token().type == token::TokenType::Semicolon){
                    l.get_token();
                }
            }
            check_token_type(l.get_token(), token::TokenType::RBrace);
            if(tag_type.value == "struct"){
                tags.push_back(std::make_unique<ast::TagDecl>(tag_type, type::StructType(ident, members, indices)));
                return std::make_pair(type::StructType(ident), std::move(tags));
            }else{
                tags.push_back(std::make_unique<ast::TagDecl>(tag_type, type::UnionType(ident, members, indices)));
                return std::make_pair(type::UnionType(ident), std::move(tags));
            }
        }else{
            auto tags = std::vector<std::unique_ptr<ast::TypeDecl>>{};
            if(tag_type.value == "struct"){
                return std::make_pair(type::StructType(ident),std::move(tags));
            }else{
                return std::make_pair(type::UnionType(ident),std::move(tags));
            }
        }
    }

    void parse_storage_specifier(std::optional<type::SSpecifier>& specifier, const token::Token& tok){
        if(specifier.has_value()){
            if(tok.value == "_Thread_local"){
                if(specifier.value() == type::SSpecifier::Extern){
                    specifier = type::SSpecifier::Thread_local_extern;
                    return;
                }
                if(specifier.value() == type::SSpecifier::Static){
                    specifier = type::SSpecifier::Thread_local_static;
                    return;
                }
            }
            if(specifier.value() == type::SSpecifier::Thread_local){
                if(tok.value == "extern"){
                    specifier = type::SSpecifier::Thread_local_extern;
                    return;
                }
                if(tok.value == "static"){
                    specifier = type::SSpecifier::Thread_local_static;
                    return;
                }
            }
            throw parse_error::ParseError("Can only have at most one storage specifier, except for _Thread_local with static or extern",tok);
        }else{
            specifier = type::get_storage_specifier(tok.value);
        }
    }
}//namespace


std::pair<type::CType,std::vector<std::unique_ptr<ast::TypeDecl>>> parse_specifiers(lexer::Lexer& l){
    auto type_specifier_list = std::multiset<std::string>{};
    std::optional<type::CType> base_type = std::nullopt;
    auto tags = std::vector<std::unique_ptr<ast::TypeDecl>>{};

    auto next_tok = l.peek_token();
    auto storage_specifier = std::optional<type::SSpecifier>{std::nullopt};
    auto type_qualifiers = std::unordered_set<type::TQualifier>{};
    while(type::is_specifier(next_tok.value) || next_tok.type == token::TokenType::Identifier){
        if(next_tok.type == token::TokenType::Identifier){
            //Handle typedef specifier
            if(true){//TODO: Conditions for being a normal ident and not a typedef name
                break;
            }
            //base_type = TypedefType(next_tok.value);
            if(type_specifier_list.size() > 0){
                throw parse_error::ParseError("Cannot have typedef name with other type specifiers",next_tok);
            }
        }
        //If it's not an identifier, we know we can get the token since it must be a specifier
        l.get_token();
        if(type::is_type_specifier(next_tok.value)){
            if(base_type.has_value()){
                throw parse_error::ParseError("Invalid collection of type specifiers",next_tok);
            }
            if(next_tok.value == "struct" || next_tok.value == "union" || next_tok.value == "enum"){
                if(type_specifier_list.size() > 0){
                    throw parse_error::ParseError("Cannot have struct, union, or enum with other type specifiers",next_tok);
                }
                auto pair = parse_tag_specifiers(l, next_tok);
                base_type = std::move(pair.first);
                for(auto& t : pair.second){
                    tags.push_back(std::move(t));
                }
            }else{
                type_specifier_list.insert(next_tok.value);
            }
        }
        if(type::is_storage_specifier(next_tok.value)){
            parse_storage_specifier(storage_specifier, next_tok);
        }
        if(type::is_type_qualifier(next_tok.value)){
            type_qualifiers.insert(type::get_type_qualifier(next_tok.value));
        }
        next_tok = l.peek_token();
    }
    if(!base_type.has_value()){
        try{
            base_type = convert_type_specifiers(type_specifier_list);
        }catch(std::runtime_error& e){
            throw sem_error::TypeError(e.what(), l.peek_token());
        }
    }
    if(storage_specifier.has_value()){
        base_type.value().add_storage_specifier(storage_specifier.value());
    }
    for(auto& tq : type_qualifiers){
        base_type.value().add_type_qualifier(tq);
    }
    return std::make_pair(base_type.value(),std::move(tags));
}
} //namespace parse
