#ifndef _LEXER_
#define _LEXER_
#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <string>
#include <exception>
#include <vector>
#include <deque>
#include "token.h"
#include "token_stream.h"

namespace lexer{
class Tokenizer : public TokenStream{
    std::istream& input_stream;
    std::pair<int, int> current_pos; //These are the line and col the lexer is currently reading from
    //These are the "next" tokens the user of Lexer will see
    //Confusingly, "current" comes after "next"
    //Since "current" is where the lexer is reading from
    //And "next" is the next token the user will see, which the lexer has already fully read
    std::string current_line;

    token::Token read_token_from_stream() override;
    struct TokenizingSubmethods;
    void ignore_space();
    void advance_input(std::string& current_token_value, char& c);
    Tokenizer(const Tokenizer& l) = delete; //Explicitly uncopyable
    Tokenizer operator=(const Tokenizer& l) = delete; 

    public:
    Tokenizer(std::istream& input) 
        : TokenStream(), input_stream(input), current_pos(std::make_pair(1,1)){
            auto pos = input_stream.tellg();
            std::getline(input_stream, current_line);
            input_stream.seekg(pos);
        }
    //This input stream should generally be empty
    Tokenizer(std::istream& input, std::vector<token::Token> tokens);
    std::pair<int,int> get_location() const{
        if(next_tokens.size() == 0){
            return current_pos;
        }
        return std::make_pair(next_tokens.front().loc.start_line, next_tokens.front().loc.start_col);
    }
};

class Lexer : public TokenStream{
    Tokenizer tokenizer;
    token::Token read_token_from_stream() override;
public:
    Lexer(std::istream& input) 
        : tokenizer(input) {}
    //This input stream should generally be empty
    Lexer(std::istream& input, std::vector<token::Token> tokens);
};

} //namespace lexer
#endif
