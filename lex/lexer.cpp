#include "token_stream.h"
#include "token.h"
#include "lexer.h"
#include "lexer_error.h"
#include "tokenizer.h"
#include "preprocessor.h"
#include <sstream>
#include <map>
namespace lexer{
namespace {
std::map<char, char> escape_chars = {{
    {'a','\a'},{'b','\b'},{'f','\f'},{'n','\n'},
    {'r','\r'},{'t','\t'},{'v','\v'},{'\\','\\'},
    {'\'','\''},{'\"','\"'},{'\?','\?'}
}};
std::string convert_escapes(const token::Token& tok){
    auto string = tok.value;
    auto ss = std::stringstream{};
    for(int i=0; i<string.size(); i++){
        if(string.at(i) != '\\'){
            ss << string.at(i);
        }else{
            i++;
            if(string.at(i) == '0'){
                //Hit an early null character
                ss << '\0';
                return ss.str();
            }
            if(escape_chars.find(string.at(i)) == escape_chars.end()){
                throw lexer_error::InvalidLiteral("Unknown escape sequence",string, string.at(i), std::make_pair(tok.loc.start_line, tok.loc.start_col));
            }
            ss << escape_chars.at(string.at(i));
        }
    }
    return ss.str();
}
} //anon namespace

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
        tok.value = convert_escapes(tok);
        int lookahead = 1;
        auto append = preprocessor->peek_token();
        while(append.type == token::TokenType::COMMENT
                ||append.type == token::TokenType::SPACE
                ||append.type == token::TokenType::NEWLINE
                ||append.type == token::TokenType::StrLiteral){
            if(append.type == token::TokenType::StrLiteral){
                assert(tok.value.back() == '"');
                tok.value.pop_back();
                append.value = convert_escapes(append);
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
