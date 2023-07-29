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

//Check and throw default unexpected token exception
void check_token_type(const token::Token& tok, token::TokenType type){
    if(tok.type != type){
        throw parse_error::ParseError("Expected " + token::string_name(type), tok);
    }
}
//Definitions for parsing methods

std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && type::is_specifier(next_token.value)){
        return parse_decl_list(l);
    }
    return parse_stmt(l);
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
