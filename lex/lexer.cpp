#include "token_stream.h"
#include "token.h"
#include "lexer.h"
#include "tokenizer.h"
#include "preprocessor.h"
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

Lexer::Lexer(std::istream& input){
    tokenizer = std::make_unique<Tokenizer>(input);
    assert(tokenizer && "Failed to construct tokenizer");
    preprocessor = std::make_unique<Preprocessor>(*tokenizer);
}
Lexer::~Lexer() = default;

token::Token Lexer::read_token_from_stream() {
    //Translation steps 6 and 7
    auto tok = preprocessor->get_token();
    while(tok.type == token::TokenType::COMMENT
            ||tok.type == token::TokenType::SPACE
            ||tok.type == token::TokenType::NEWLINE){
        tok = preprocessor->get_token();
    }
    if(tok.type == token::TokenType::StrLiteral){
        int lookahead = 1;
        auto append = preprocessor->peek_token();
        while(append.type == token::TokenType::COMMENT
                ||append.type == token::TokenType::SPACE
                ||append.type == token::TokenType::NEWLINE
                ||append.type == token::TokenType::StrLiteral){
            if(append.type == token::TokenType::StrLiteral){
                assert(tok.value.back() == '"');
                tok.value.pop_back();
                assert(append.value.front() == '"');
                tok.value += append.value.substr(1);
                tok.loc.end_line = append.loc.end_line;
                tok.loc.end_col = append.loc.end_col;
            }
            lookahead++;
            append = preprocessor->peek_token(lookahead);
        }
        for(int i=1; i<lookahead; i++){
            preprocessor->get_token();
        }
    }
    return tok;
}
} //namespace lexer
