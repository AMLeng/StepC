#ifndef _LEXER_
#define _LEXER_
#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <string>
#include <exception>
#include "location.h"

namespace lexer{

struct Token{
    enum class TokenType{
        LBrace, RBrace, LParen, RParen, LBrack, RBrack, Semicolon, Period,
        Keyword, Identifier, IntegerLiteral, BitwiseNot, Not, Minus, Plus, Mult, Div,
        END, COMMENT
    };

    TokenType type;
    std::string value;
    location::Location loc;
    static Token make_end_token(std::pair<int, int> position){
        location::Location loc = {position.first, position.second, position.first, position.second};
        return Token{Token::TokenType::END, "", loc};
    }
    static std::string string_name(TokenType type){
        switch(type){
            case TokenType:: LBrace:
                return "left brace '{'";
            case TokenType:: RBrace:
                return "right brace '}'";
            case TokenType:: LParen:
                return "left parenthesis '('";
            case TokenType:: RParen:
                return "right parenthesis ')'";
            case TokenType:: LBrack:
                return "left bracket '['";
            case TokenType:: RBrack:
                return "right bracket ']'";
            case TokenType:: Semicolon:
                return "semicolon";
            case TokenType:: Period:
                return "period";
            case TokenType:: Keyword:
                return "keyword";
            case TokenType:: Identifier:
                return "Identifier";
            case TokenType:: IntegerLiteral:
                return "integer literal";
            case TokenType:: COMMENT:
                return "comment";
            case TokenType:: END:
                return "end of input stream";
            default:
                //Annotation or g++ complains
                __builtin_unreachable();
                assert(false);
                return "unknown type";
        }
    }
};

class Lexer{
        std::istream& input_stream;
        std::exception_ptr lexer_error; 
        std::pair<int, int> current_pos; //These are the line and col the lexer is currently reading from
        Token next_token; //This is the "next" token the user of Lexer will see
        //Confusingly, "current" comes after "next"
        //Since "current" is where the lexer is reading from
        //And "next" is the next token the user will see, which the lexer has already fully read

        Token read_token_from_stream();
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
                next_token = Token::make_end_token(current_pos);
                lexer_error = std::current_exception();
            }
        }

        Token peek_token() const{
            if(lexer_error){
                std::rethrow_exception(lexer_error);
            }
            return next_token;
        }
        Token get_token(){
            auto current = peek_token();
            try{
                next_token = read_token_from_stream();
            }
            catch(...){
                next_token = Token::make_end_token(current_pos);
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
