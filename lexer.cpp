#include "lexer.h"
#include "lexer_error.h"
#include <cctype>
#include <string>
#include <exception>
#include <utility>
namespace lexer{
namespace{
bool is_keyword(const std::string& word){
    return word == "int"
        || word == "return";
}

token::Token create_token(token::TokenType type, std::string value, std::pair<int, int> tok_start, std::pair<int, int> tok_end){
    location::Location loc = {tok_start.first, tok_start.second, tok_end.first, tok_end.second};
    return token::Token{type, value, loc};
}
} //namespace

void Lexer::ignore_space(){
    char next_char = input_stream.peek();
    while(std::isspace(next_char)){
        if(next_char == '\n'){
            this->current_pos.first++;
            this->current_pos.second = 1;
        }else{
            this->current_pos.second++;
        }
        input_stream.ignore(1);
        next_char = input_stream.peek();
    }
}


void Lexer::advance_input(std::string& already_read, char& next_to_see){
    if(next_to_see == '\n'){
        this->current_pos.first++;
        this->current_pos.second = 1;
    }else{
        this->current_pos.second++;
    }
    already_read.push_back(next_to_see);
    input_stream.ignore(1);
    next_to_see = input_stream.peek();
}

token::Token Lexer::read_token_from_stream(){
    ignore_space();

    std::pair<int, int> starting_position = current_pos;
    char c = input_stream.peek();
    std::string token_value = "";

    if(c== EOF){
        return token::Token::make_end_token(current_pos);
    }

    //Handle keywords and identifiers
    if(std::isalpha(c)){
        do{
            advance_input(token_value, c);
        }while(std::isalpha(c) || std::isdigit(c) || c == '_');

        if(is_keyword(token_value)){
            return create_token(token::TokenType::Keyword, token_value, starting_position, current_pos);
        }
        return create_token(token::TokenType::Identifier, token_value, starting_position, current_pos);
    }//Finished handling keywords and identifiers

    //Handle all punctuation
    if(c == '('){
        advance_input(token_value, c);
        return create_token(token::TokenType::LParen, token_value, starting_position, current_pos);
    }
    if(c == ')'){
        advance_input(token_value, c);
        return create_token(token::TokenType::RParen, token_value, starting_position, current_pos);
    }
    if(c=='{'){
        advance_input(token_value, c);
        return create_token(token::TokenType::LBrace, token_value, starting_position, current_pos);
    }
    if(c =='}'){
        advance_input(token_value, c);
        return create_token(token::TokenType::RBrace, token_value, starting_position, current_pos);
    }
    if(c ==';'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Semicolon, token_value, starting_position, current_pos);
    }

    //Handle ints
    if(std::isdigit(c)){
        if(c == '0'){
            advance_input(token_value, c);
            if(c == 'x' || c == 'X'){
                //Hex integer
                advance_input(token_value, c);
                if(!std::isxdigit(c)){
                    throw lexer_error::InvalidLiteral("Invalid hexadecimal integer", token_value, c, starting_position);
                }
                while(std::isxdigit(c)){
                    advance_input(token_value, c);
                }
                if(c != '.' && c != 'p'){
                    //Suffix handling
                    bool u_read = false;
                    if(c == 'u' || c == 'U'){
                        advance_input(token_value, c);
                    }
                    if(c == 'l' || c == 'L'){
                        advance_input(token_value, c);
                        //This ensures we only consider ll and LL
                        //Not mixed suffixes like lL or Ll
                        if(c == token_value.back()){
                            advance_input(token_value,c);
                        }
                    }
                    if(c == 'u' || c == 'U' && !u_read){
                        advance_input(token_value, c);
                    }
                    return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
                }else{
                    //Hex floating point
                    throw lexer_error::NotImplemented("Floating-point literals not yet implemented", token_value, c, starting_position);
                }
            }else{
                //Octal integer
                //We use a while loop and not a do-while loop
                //Since "0" is a perfectly valid octal literal
                while(std::isdigit(c)){
                    if(c == '8' || c == '9'){
                        throw lexer_error::InvalidLiteral("Invalid octal integer", token_value, c, starting_position);
                    }
                    advance_input(token_value, c);
                }
                if(c != '.'){
                    //Suffix handling
                    bool u_read = false;
                    if(c == 'u' || c == 'U'){
                        advance_input(token_value, c);
                    }
                    if(c == 'l' || c == 'L'){
                        advance_input(token_value, c);
                        //This ensures we only consider ll and LL
                        //Not mixed suffixes like lL or Ll
                        if(c == token_value.back()){
                            advance_input(token_value,c);
                        }
                    }
                    if(c == 'u' || c == 'U' && !u_read){
                        advance_input(token_value, c);
                    }
                    return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
                }else{
                    throw lexer_error::InvalidLiteral("Octal floating-point numbers not allowed", token_value, c, starting_position);
                }
            }
        }
        //Decimal integer
        while(std::isdigit(c)){
            advance_input(token_value,c);
        }
        if(c != '.' && c != 'e'){
            //Suffix handling
            bool u_read = false;
            if(c == 'u' || c == 'U'){
                advance_input(token_value, c);
            }
            if(c == 'l' || c == 'L'){
                advance_input(token_value, c);
                //This ensures we only consider ll and LL
                //Not mixed suffixes like lL or Ll
                if(c == token_value.back()){
                    advance_input(token_value,c);
                }
            }
            if(c == 'u' || c == 'U' && !u_read){
                advance_input(token_value, c);
            }
            return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
        }else{
            //Decimal floating point
            throw lexer_error::NotImplemented("Floating-point literals not yet implemented", token_value, c, starting_position);
        }
    } //Finished handling ints

    //Handle unary operators
    if(c=='-'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Minus, token_value, starting_position, current_pos);
    }
    if(c =='~'){
        advance_input(token_value, c);
        return create_token(token::TokenType::BitwiseNot, token_value, starting_position, current_pos);
    }
    if(c =='!'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Not, token_value, starting_position, current_pos);
    }

    //Handle binary operators, aside from "minus" which was taken care of above
    if(c=='+'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Plus, token_value, starting_position, current_pos);
    }
    if(c =='*'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Mult, token_value, starting_position, current_pos);
    }
    //Note that we have already taken care of comments
    if(c =='/'){
        advance_input(token_value, c);
        return create_token(token::TokenType::Div, token_value, starting_position, current_pos);
    }

    //Other cases/not implemented yet/not parsable
    throw lexer_error::UnknownInput("Unknown input", token_value, c, starting_position);
    return create_token(token::TokenType::END, token_value, starting_position, current_pos);
}

} //namespace lexer
