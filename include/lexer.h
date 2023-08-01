#ifndef _LEXER_
#define _LEXER_
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <exception>
#include <vector>
#include <deque>
#include "token.h"
#include "token_stream.h"
namespace lexer{

class Tokenizer;
class Preprocessor;
class Lexer : public TokenStream{
    std::unique_ptr<Tokenizer> tokenizer;
    std::unique_ptr<Preprocessor> preprocessor;
    token::Token read_token_from_stream() override;
public:
    Lexer(std::istream& input);
    ~Lexer();
};

} //namespace lexer
#endif
