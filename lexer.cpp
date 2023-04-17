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

Token create_token(Token::TokenType type, std::string value, std::pair<int, int> tok_start, std::pair<int, int> tok_end){
    location::Location loc = {tok_start.first, tok_start.second, tok_end.first, tok_end.second};
    return Token{type, value, loc};
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

Token Lexer::read_token_from_stream(){
    ignore_space();

    std::pair<int, int> starting_position = current_pos;
    char c = input_stream.peek();
    std::string token_value = "";

    if(c== EOF){
        return Token::make_end_token(current_pos);
    }

    //Handle keywords and identifiers
    if(std::isalpha(c)){
        do{
            advance_input(token_value, c);
        }while(std::isalpha(c));

        if(is_keyword(token_value)){
            return create_token(Token::TokenType::Keyword, token_value, starting_position, current_pos);
        }
        return create_token(Token::TokenType::Identifier, token_value, starting_position, current_pos);
    }//Finished handling keywords and identifiers

    //Handle all punctuation
    if(c == '('){
        advance_input(token_value, c);
        return create_token(Token::TokenType::LParen, token_value, starting_position, current_pos);
    }
    if(c == ')'){
        advance_input(token_value, c);
        return create_token(Token::TokenType::RParen, token_value, starting_position, current_pos);
    }
    if(c=='{'){
        advance_input(token_value, c);
        return create_token(Token::TokenType::LBrace, token_value, starting_position, current_pos);
    }
    if(c =='}'){
        advance_input(token_value, c);
        return create_token(Token::TokenType::RBrace, token_value, starting_position, current_pos);
    }
    if(c ==';'){
        advance_input(token_value, c);
        return create_token(Token::TokenType::Semicolon, token_value, starting_position, current_pos);
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
                    return create_token(Token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
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
                    return create_token(Token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
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
            return create_token(Token::TokenType::IntegerLiteral, token_value, starting_position, current_pos);
        }else{
            //Decimal floating point
            throw lexer_error::NotImplemented("Floating-point literals not yet implemented", token_value, c, starting_position);
        }
    } //Finished handling ints

    //Other cases/not implemented yet/not parsable
    throw lexer_error::UnknownInput("Unknown input", token_value, c, starting_position);
    return create_token(Token::TokenType::END, token_value, starting_position, current_pos);
}

} //namespace lexer
