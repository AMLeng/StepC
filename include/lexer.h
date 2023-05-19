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

namespace lexer{

class Lexer{
        std::istream& input_stream;
        std::pair<int, int> current_pos; //These are the line and col the lexer is currently reading from
        std::deque<token::Token> next_tokens; 
        //These are the "next" tokens the user of Lexer will see
        //Confusingly, "current" comes after "next"
        //Since "current" is where the lexer is reading from
        //And "next" is the next token the user will see, which the lexer has already fully read
        std::string current_line;

        token::Token read_token_from_stream() ;
        //Reads the next token to next_token, unless it produces an error
        //In which case set next_token to a token of type END
        //And produse an error
        struct LexingSubmethods;
        void ignore_space();
        void advance_input(std::string& current_token_value, char& c);
        Lexer(const Lexer& l) = delete; //Explicitly uncopyable
        Lexer operator=(const Lexer& l) = delete; 

    public:
        Lexer(std::istream& input) 
            : input_stream(input), current_pos(std::make_pair(1,1)), next_tokens(){
            auto pos = input_stream.tellg();
            std::getline(input_stream, current_line);
            input_stream.seekg(pos);
        }
        token::Token peek_token(int n = 1) {
            while(n >next_tokens.size()){
                next_tokens.push_back(read_token_from_stream());
            }
            return next_tokens.at(n-1);
        }
        token::Token get_token(){
            auto current = peek_token();
            next_tokens.pop_front();
            return current;
        }
        std::pair<int,int> get_location() const{
            if(next_tokens.size() == 0){
                return current_pos;
            }
            return std::make_pair(next_tokens.front().loc.start_line, next_tokens.front().loc.start_col);
        }
};

} //namespace lexer
#endif
