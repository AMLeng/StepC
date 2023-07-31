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
} //anon namespace

//Check and throw default unexpected token exception
void check_token_type(const token::Token& tok, token::TokenType type){
    if(tok.type != type){
        throw parse_error::ParseError("Expected " + token::string_name(type), tok);
    }
}
//Definitions for parsing methods

std::unique_ptr<ast::AmbiguousBlock> parse_ambiguous_block(lexer::Lexer& l){
    auto next= l.peek_token();
    auto toks = std::vector<token::Token>{next};
    do{
        l.get_token();
        next = l.peek_token();
        toks.push_back(next);
    }while(next.type != token::TokenType::END && next.type != token::TokenType::Semicolon);
    return std::make_unique<ast::AmbiguousBlock>(std::move(toks));
}

std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l){
    if(type::is_specifier(l.peek_token().value)){
        return parse_decl_list(l);
    }else if(l.peek_token().type == token::TokenType::Identifier && l.peek_token(2).type != token::TokenType::Colon){
        //Identifier followed by a colon is the one non-expr non-decl block item
        return parse_ambiguous_block(l);
    }else{
        return parse_stmt(l);
    }
}

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    type::CType::reset_tables();
    auto next = l.peek_token();
    auto global_decls = std::vector<std::unique_ptr<ast::ExtDecl>>{};
    while(next.type != token::TokenType::END){
        global_decls.push_back(parse_ext_decl(l));
        next = l.peek_token();
    }
    return std::make_unique<ast::Program>(std::move(global_decls));
}

} //namespace parse
