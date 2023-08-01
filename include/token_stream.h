#ifndef _TOKEN_STREAM_
#define _TOKEN_STREAM_
#include <deque>
#include <vector>
#include "token.h"

namespace lexer{
class TokenStream{
    virtual token::Token read_token_from_stream();
protected:
    std::deque<token::Token> next_tokens; 
public:
    TokenStream() : next_tokens() {}
    TokenStream(std::vector<token::Token> tokens);
    virtual ~TokenStream();
    token::Token get_token(){
        auto current = peek_token();
        next_tokens.pop_front();
        return current;
    }
    token::Token peek_token(int n = 1) {
        while(n >next_tokens.size()){
            next_tokens.push_back(read_token_from_stream());
        }
        return next_tokens.at(n-1);
    }
};
} //namespace lexer
#endif
