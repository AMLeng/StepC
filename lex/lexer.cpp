#include "token_stream.h"
#include "token.h"
#include "lexer.h"
namespace lexer{
token::Token TokenStream::read_token_from_stream(){
    return token::Token::make_end_token(std::make_pair(-1,-1));
}
TokenStream::~TokenStream() = default;
TokenStream::TokenStream(std::vector<token::Token> tokens){
    for(const auto& t : tokens){
        this->next_tokens.push_back(t);
    }
}

token::Token Lexer::read_token_from_stream() {
    auto tok = tokenizer.get_token();
    while(tok.type == token::TokenType::COMMENT){
        tok = tokenizer.get_token();
    }
    return tok;
}
} //namespace lexer
