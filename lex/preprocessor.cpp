#include "preprocessor.h"
namespace lexer{
token::Token Preprocessor::read_token_from_stream(){
    if(tokens.size() == 0){
        tokens.push_back(EnhancedToken(stream.get_token()));
    }
    auto tok = tokens.front();
    if(tok.colored || tok.base.type != token::TokenType::Identifier){
        tokens.pop_front();
        return tok.base;
    }
    //Put preprocessing here
    tokens.pop_front();
    return tok.base;
}

}//namespace lexer
