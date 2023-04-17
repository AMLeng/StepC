#ifndef _LEXER_
#define _LEXER_
#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <string>
#include <exception>
#include "location.h"
#include "token.h"

namespace lexer{


class Lexer{
        std::istream& input_stream;
        std::exception_ptr lexer_error; 
        std::pair<int, int> current_pos; //These are the line and col the lexer is currently reading from
        token::Token next_token; //This is the "next" token the user of Lexer will see
        //Confusingly, "current" comes after "next"
        //Since "current" is where the lexer is reading from
        //And "next" is the next token the user will see, which the lexer has already fully read

        token::Token read_token_from_stream();
        //Reads the next token to next_token, unless it produces an error
        //In which case set next_token to a token of type END
        //And produse an error
        void update_next_token();
        void advance_input(std::string& current_token_value, char& c);
        void ignore_space();
        Lexer(const Lexer& l) = delete; //Explicitly uncopyable
        Lexer operator=(const Lexer& l) = delete; 

    public:
        Lexer(std::istream& input) : input_stream(input), lexer_error(nullptr), current_pos(std::make_pair(1,1)) {
            try{
                next_token = read_token_from_stream();
            }
            catch(...){
                next_token = token::Token::make_end_token(current_pos);
                lexer_error = std::current_exception();
            }
        }

        token::Token peek_token() const{
            if(lexer_error){
                std::rethrow_exception(lexer_error);
            }
            return next_token;
        }
        token::Token get_token(){
            auto current = peek_token();
            try{
                next_token = read_token_from_stream();
            }
            catch(...){
                next_token = token::Token::make_end_token(current_pos);
                lexer_error = std::current_exception();
            }
            return current;
        }
        std::pair<int,int> get_location() const{
            return std::make_pair(next_token.loc.start_line, next_token.loc.start_col);
        }
};

} //namespace lexer
#endif
