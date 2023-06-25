#include "lexer.h"
#include "lexer_error.h"
#include <cctype>
#include <map>
#include <string>
#include <exception>
#include <utility>
namespace lexer{
namespace{
bool is_keyword(const std::string& word){
    return word == "return"
        || word == "if"
        || word == "else"
        || word == "for"
        || word == "do"
        || word == "while"
        || word == "continue"
        || word == "break"
        || word == "goto"
        || word == "switch"
        || word == "case"
        || word == "default"
        || word == "void"
        || word == "char"
        || word == "short"
        || word == "int"
        || word == "long"
        || word == "float"
        || word == "double"
        || word == "signed"
        || word == "unsigned"
        || word == "_Bool";
}

token::Token create_token(token::TokenType type, std::string value, std::pair<int, int> tok_start, std::pair<int, int> tok_end, const std::string& source){
    location::Location loc = {tok_start.first, tok_start.second, tok_end.first, tok_end.second};
    return token::Token{type, value, loc, source};
}
const std::map<char, token::TokenType> followed_by_eq = {{
    {'!',token::TokenType::NEqual},
    {'>',token::TokenType::GEq},
    {'<',token::TokenType::LEq},
    {'=',token::TokenType::Equal},
    {'+',token::TokenType::PlusAssign},
    {'-',token::TokenType::MinusAssign},
    {'*',token::TokenType::MultAssign},
    {'/',token::TokenType::DivAssign},
    {'%',token::TokenType::ModAssign},
    {'^',token::TokenType::BXAssign},
    {'|',token::TokenType::BOAssign},
    {'&',token::TokenType::BAAssign},
}};

const std::map<char, token::TokenType> single_char_tokens = {{
    {'(',token::TokenType::LParen},
    {')',token::TokenType::RParen},
    {'{',token::TokenType::LBrace},
    {'}',token::TokenType::RBrace},
    {';',token::TokenType::Semicolon},
    {',',token::TokenType::Comma},
    {'~',token::TokenType::BitwiseNot},
    {'&',token::TokenType::Amp},
    {'|',token::TokenType::BitwiseOr},
    {'^',token::TokenType::BitwiseXor},
    {'!',token::TokenType::Not},
    {'-',token::TokenType::Minus},
    {'+',token::TokenType::Plus},
    {'*',token::TokenType::Star},
    {'/',token::TokenType::Div},
    {'%',token::TokenType::Mod},
    {'=',token::TokenType::Assign},
    {'<',token::TokenType::Less},
    {'>',token::TokenType::Greater},
    {':',token::TokenType::Colon},
    {'?',token::TokenType::Question},
    {'[',token::TokenType::LBrack},
    {']',token::TokenType::RBrack},
}};

} //namespace

void Lexer::ignore_space(){
    char next_char = input_stream.peek();
    while(std::isspace(next_char)){
        input_stream.ignore(1);
        if(next_char == '\n'){
            this->current_pos.first++;
            this->current_pos.second = 1;
            auto pos = input_stream.tellg();
            std::getline(input_stream, current_line);
            input_stream.seekg(pos);
        }else{
            this->current_pos.second++;
        }
        next_char = input_stream.peek();
    }
}


void Lexer::advance_input(std::string& already_read, char& next_to_see){
    input_stream.ignore(1);
    if(next_to_see == '\n'){
        this->current_pos.first++;
        this->current_pos.second = 1;
        auto pos = input_stream.tellg();
        std::getline(input_stream, current_line);
        input_stream.seekg(pos);
    }else{
        this->current_pos.second++;
    }
    already_read.push_back(next_to_see);
    next_to_see = input_stream.peek();
}

struct Lexer::LexingSubmethods{
    static token::Token lex_keyword_ident(Lexer& l);
    static token::Token lex_numeric_literals(Lexer& l);

    static void handle_int_literal_suffix(Lexer& l, char& c, std::string& token_value);
    static token::Token lex_hex_fractional(Lexer& l, char& c, std::string& token_value, const std::pair<int,int>& starting_position);
    static token::Token lex_decimal_fractional(Lexer& l, char& c, std::string& token_value, const std::pair<int,int>& starting_position);
};

token::Token Lexer::LexingSubmethods::lex_keyword_ident(Lexer& l){
    std::pair<int, int> starting_position = l.current_pos;
    char c = l.input_stream.peek();
    std::string token_value = "";
    assert(std::isalpha(c) || c == '_');
    do{
        l.advance_input(token_value, c);
    }while(std::isalpha(c) || std::isdigit(c) || c == '_');

    if(is_keyword(token_value)){
        return create_token(token::TokenType::Keyword, token_value, starting_position, l.current_pos, l.current_line);
    }
    return create_token(token::TokenType::Identifier, token_value, starting_position, l.current_pos, l.current_line);
}


token::Token Lexer::read_token_from_stream() {
    ignore_space();

    char c = input_stream.peek();

    //Straightforward cases (token type determined by first character)
    if(c== EOF){
        return token::Token::make_end_token(current_pos);
    }
    if(std::isalpha(c) || c == '_'){
        return Lexer::LexingSubmethods::lex_keyword_ident(*this);
    }
    //Handle ints and floats that start with a digit
    if(std::isdigit(c)){
        return Lexer::LexingSubmethods::lex_numeric_literals(*this);
    }

    //More complicated cases
    std::pair<int, int> starting_position = current_pos;
    std::string token_value = "";
    if(c == '.'){
        advance_input(token_value, c);
        if(c == '.'){
            advance_input(token_value, c);
            if(c == '.'){
                advance_input(token_value, c);
                return create_token(token::TokenType::Ellipsis, token_value, starting_position, current_pos, current_line);
            }
            throw lexer_error::UnknownInput("Unknown input", token_value, c, starting_position);
        }
        if(std::isdigit(c)){
            return Lexer::LexingSubmethods::lex_decimal_fractional(*this, c, token_value, starting_position);
        }
        if(std::isalpha(c)){
            return create_token(token::TokenType::Period, token_value, starting_position, current_pos, current_line);
        }
    }
    if (c == '<' || c == '>'){
        advance_input(token_value, c);
        if(c == token_value.back()){
            if(c == '<'){
                advance_input(token_value, c);
                if(c == '='){
                    advance_input(token_value, c);
                    return create_token(token::TokenType::LSAssign, token_value, starting_position, current_pos, current_line);
                }
                return create_token(token::TokenType::LShift, token_value, starting_position, current_pos, current_line);
            }
            if(c== '>'){
                advance_input(token_value, c);
                if(c == '='){
                    advance_input(token_value, c);
                    return create_token(token::TokenType::RSAssign, token_value, starting_position, current_pos, current_line);
                }
                return create_token(token::TokenType::RShift, token_value, starting_position, current_pos, current_line);
            }
        }
        if(c == '='){
            auto type = followed_by_eq.at(token_value.back());
            advance_input(token_value, c);
            return create_token(type, token_value, starting_position, current_pos, current_line);
        }
        auto type = single_char_tokens.at(token_value.back());
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }
    if (c == '&' || c == '|'){
        advance_input(token_value, c);
        if(c == token_value.back()){
            if(c == '&'){
                advance_input(token_value, c);
                return create_token(token::TokenType::And, token_value, starting_position, current_pos, current_line);
            }
            if(c== '|'){
                advance_input(token_value, c);
                return create_token(token::TokenType::Or, token_value, starting_position, current_pos, current_line);
            }
        }
        if(c == '='){
            auto type = followed_by_eq.at(token_value.back());
            advance_input(token_value, c);
            return create_token(type, token_value, starting_position, current_pos, current_line);
        }
        auto type = single_char_tokens.at(token_value.back());
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }
    if (c == '+' || c == '-'){
        advance_input(token_value, c);
        if(c == token_value.back()){
            if(c == '+'){
                advance_input(token_value, c);
                return create_token(token::TokenType::Plusplus, token_value, starting_position, current_pos, current_line);
            }
            if(c== '-'){
                advance_input(token_value, c);
                return create_token(token::TokenType::Minusminus, token_value, starting_position, current_pos, current_line);
            }
        }
        if(c == '='){
            auto type = followed_by_eq.at(token_value.back());
            advance_input(token_value, c);
            return create_token(type, token_value, starting_position, current_pos, current_line);
        }
        auto type = single_char_tokens.at(token_value.back());
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }
    if(followed_by_eq.find(c) != followed_by_eq.end()){
        advance_input(token_value, c);
        if(c == '='){
            auto type = followed_by_eq.at(token_value.back());
            advance_input(token_value, c);
            return create_token(type, token_value, starting_position, current_pos, current_line);
        }
        auto type = single_char_tokens.at(token_value.back());
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }

    //Handle all remaining single character tokens
    if(single_char_tokens.find(c) != single_char_tokens.end()){
        assert(starting_position == current_pos); //Make sure we're actually lexing a single character token
        auto type = single_char_tokens.at(c);
        advance_input(token_value, c);
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }

    //Other cases/not implemented yet/not parsable
    throw lexer_error::UnknownInput("Unknown input", token_value, c, starting_position);
    return create_token(token::TokenType::END, "", starting_position, current_pos, this->current_line);
}

token::Token Lexer::LexingSubmethods::lex_decimal_fractional(Lexer& l, char& c, std::string& token_value, const std::pair<int,int>& starting_position){
    //Assumes that token_value already contains the integral part, if any
    if(c == '.'){
        l.advance_input(token_value, c);
    }
    while(std::isdigit(c)){
        l.advance_input(token_value, c);
    }
    if(c == 'e' || c == 'E'){
        l.advance_input(token_value, c);
        if(c == '+' || c == '-'){
            l.advance_input(token_value, c);
        }
        if(!std::isdigit(c)){
            throw lexer_error::InvalidLiteral(
                    "Decimal floating point in scientific notation missing exponent", token_value, c, starting_position);
        }
        while(std::isdigit(c)){
            l.advance_input(token_value,c);
        }
    }
    //Suffix handling
    if(c == 'f' || c == 'F' || c == 'l' || c == 'L'){
        l.advance_input(token_value, c);
    }
    return create_token(token::TokenType::FloatLiteral, token_value, starting_position, l.current_pos, l.current_line);
}
token::Token Lexer::LexingSubmethods::lex_hex_fractional(Lexer& l, char& c, std::string& token_value, const std::pair<int,int>& starting_position){
    //Assumes that token_value already contains the integral part, if any
    if(c == '.'){
        l.advance_input(token_value, c);
    }
    while(std::isxdigit(c)){
        l.advance_input(token_value, c);
    }
    if(c != 'p' && c != 'P'){
        throw lexer_error::InvalidLiteral(
                "Hexadecimal floating point values are required to have an exponent", token_value, c, starting_position);
    }
    l.advance_input(token_value, c);
    if(c == '+' || c == '-'){
        l.advance_input(token_value, c);
    }
    if(!std::isdigit(c)){
        throw lexer_error::InvalidLiteral(
                "Hexadecimal floating point values are required to have a decimal exponent", token_value, c, starting_position);
    }
    while(std::isdigit(c)){
        l.advance_input(token_value,c);
    }
    //Suffix handling
    if(c == 'f' || c == 'F' || c == 'l' || c == 'L'){
        l.advance_input(token_value, c);
    }
    return create_token(token::TokenType::FloatLiteral, token_value, starting_position, l.current_pos, l.current_line);
}

void Lexer::LexingSubmethods::handle_int_literal_suffix(Lexer& l, char& c, std::string& token_value){
    bool u_read = false;
    if(c == 'u' || c == 'U'){
        l.advance_input(token_value, c);
    }
    if(c == 'l' || c == 'L'){
        l.advance_input(token_value, c);
        //This ensures we only consider ll and LL
        //Not mixed suffixes like lL or Ll
        if(c == token_value.back()){
            l.advance_input(token_value,c);
        }
    }
    if(c == 'u' || c == 'U' && !u_read){
        l.advance_input(token_value, c);
    }
}

token::Token Lexer::LexingSubmethods::lex_numeric_literals(Lexer& l){
    char c = l.input_stream.peek();
    std::string token_value = "";
    std::pair<int, int> starting_position = l.current_pos;
    if(c == '0'){
        l.advance_input(token_value, c);
        if(c == 'x' || c == 'X'){
            l.advance_input(token_value, c);
            if(!std::isxdigit(c)){
                throw lexer_error::InvalidLiteral("Invalid hexadecimal literal", token_value, c, starting_position);
            }
            while(std::isxdigit(c)){
                l.advance_input(token_value, c);
            }
            if(c != '.' && c != 'p' && c != 'P'){
                //Hex Integer
                Lexer::LexingSubmethods::handle_int_literal_suffix(l,c,token_value);
                return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
            }else{
                //Hex floating point
                return Lexer::LexingSubmethods::lex_hex_fractional(l,c,token_value, starting_position);
            }
        }else{
            //Octal integer or decimal float
            bool non_octal_digit = false;
            while(std::isdigit(c)){
                if(c == '8' || c == '9'){
                    non_octal_digit = true;
                }
                l.advance_input(token_value, c);
            }
            if(c != '.' && c != 'e' && c != 'E'){
                //Octal integer
                if(non_octal_digit){
                    throw lexer_error::InvalidLiteral("Invalid octal integer", token_value, c, starting_position);
                }
                Lexer::LexingSubmethods::handle_int_literal_suffix(l,c,token_value);
                return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
            }else{
                //Decimal float with a leading 0
                return Lexer::LexingSubmethods::lex_decimal_fractional(l,c,token_value, starting_position);
            }
        }
    }
    while(std::isdigit(c)){
        l.advance_input(token_value,c);
    }
    if(c != '.' && c != 'e' && c != 'E'){
        Lexer::LexingSubmethods::handle_int_literal_suffix(l,c,token_value);
        return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
    }else{
        //Decimal floating point
        return Lexer::LexingSubmethods::lex_decimal_fractional(l,c,token_value, starting_position);
    }
}


} //namespace lexer
