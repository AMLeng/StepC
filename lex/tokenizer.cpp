#include "lexer.h"
#include "type.h"
#include "tokenizer.h"
#include "lexer_error.h"
#include <cctype>
#include <map>
#include <string>
#include <exception>
#include <utility>
namespace lexer{
namespace{
std::pair<int, int> starting_position = {1,1};
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
        || word == "sizeof"
        || word == "_Alignof"
        || type::is_specifier(word);
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

void Tokenizer::custom_ignore_one(){
    auto next_to_see = input_stream.peek();
    input_stream.ignore(1); 
    //If we start a trigraph, ignore two more
    if(next_to_see == '?'){
        auto pos = input_stream.tellg();
        if(input_stream.peek() == '?'){
            input_stream.ignore(1); 
            next_to_see = input_stream.get(); 
            switch(next_to_see){
                case '=':
                case '/':
                case '\'':
                case '(':
                case ')':
                case '!':
                case '<':
                case '>':
                case '-':
                    current_pos.second += 2; 
                    return;
                default:
                    input_stream.seekg(pos);
                    return;
            }
        }
    }
}

char Tokenizer::custom_peek(){
    auto next_to_see = input_stream.peek();
    //This block takes care of trigraphs
    if(next_to_see == '?'){
        auto pos = input_stream.tellg();
        input_stream.ignore(1); 
        if(input_stream.peek() == '?'){
            input_stream.ignore(1); 
            next_to_see = input_stream.get(); 
            current_pos.second += 2; 
            switch(next_to_see){
                case '=':
                    next_to_see = '#';
                    break;
                case '/':
                    next_to_see = '\\';
                    break;
                case '\'':
                    next_to_see = '^';
                    break;
                case '(':
                    next_to_see = '[';
                    break;
                case ')':
                    next_to_see = ']';
                    break;
                case '!':
                    next_to_see = '|';
                    break;
                case '<':
                    next_to_see = '{';
                    break;
                case '>':
                    next_to_see = '}';
                    break;
                case '-':
                    next_to_see = '~';
                    break;
                default:
                    next_to_see = '?';
                    break;
            }
        }
        input_stream.seekg(pos);
        return next_to_see;
    }

    //This block takes care of line splicing
    if(next_to_see =='\\'){
        //If backslash, must check if next is newline, in which case 
        auto pos = input_stream.tellg();
        input_stream.ignore(1); //ignore backslash
        if(input_stream.peek() == '\n'){
            input_stream.ignore(1); //ignore newline
            next_to_see = input_stream.peek();
            pos = input_stream.tellg();
            current_pos.first++;
            current_pos.second = 2; 
            std::getline(input_stream, current_line);
        }
        input_stream.seekg(pos);
    }
    return next_to_see;
}

//Here we pre-emptively take care of translation phases 1 and 2
void Tokenizer::advance_input(std::string& already_read, char& next_to_see){
    //Note that since we perform translation phases 1 and 2 here, we can't simply just
    //get the characters in general, since next_to_see might (in the case of trigraphs)
    //be set to a character that doesn't actually appear in the source code
    custom_ignore_one(); //Behaves like input_stream.ignore(1), except also tracks trigraphs correctly
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
    next_to_see = custom_peek();
}

struct Tokenizer::TokenizingSubmethods{
    static token::Token lex_keyword_ident(Tokenizer& l);
    static token::Token lex_numeric_literals(Tokenizer& l);

    static void handle_int_literal_suffix(Tokenizer& l, char& c, std::string& token_value);
    static token::Token lex_hex_fractional(Tokenizer& l, char& c, std::string& token_value);
    static token::Token lex_decimal_fractional(Tokenizer& l, char& c, std::string& token_value);
};

token::Token Tokenizer::TokenizingSubmethods::lex_keyword_ident(Tokenizer& l){
    char c = l.custom_peek();
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


token::Token Tokenizer::read_token_from_stream() {
    starting_position = current_pos; //starting_position is a file scope global which should only ever be modified here
    char c = custom_peek();

    if(std::isspace(c)){
        //We know it's a space and not a trigraph so we just ignore it
        if(c == '\n'){
            input_stream.ignore(1);
            this->current_pos.first++;
            this->current_pos.second = 1;
            auto pos = input_stream.tellg();
            std::getline(input_stream, current_line);
            input_stream.seekg(pos);
            return create_token(token::TokenType::NEWLINE, "\n", starting_position, current_pos, current_line);
        }else{
            do{
                input_stream.ignore(1);
                this->current_pos.second++;
                c = input_stream.peek();
            }while(std::isspace(c) && c != '\n');
            return create_token(token::TokenType::SPACE, " ", starting_position, current_pos, current_line);
        }
    }

    //Straightforward cases (token type determined by first character)
    if(c== EOF){
        return token::Token::make_end_token(current_pos);
    }
    if(std::isalpha(c) || c == '_'){
        return Tokenizer::TokenizingSubmethods::lex_keyword_ident(*this);
    }
    //Handle ints and floats that start with a digit
    if(std::isdigit(c)){
        return Tokenizer::TokenizingSubmethods::lex_numeric_literals(*this);
    }

    std::string token_value = "";
    //String literals
    if(c == '"'){
        advance_input(token_value, c);
        while(c != '"'){
            if(c == '\\'){
                advance_input(token_value, c);
            }
            if(c == EOF){
                throw lexer_error::InvalidLiteral("Reached EOF in string literal", token_value, c, starting_position);
            }
            advance_input(token_value, c);
        }
        advance_input(token_value, c);
        return create_token(token::TokenType::StrLiteral, token_value, starting_position, current_pos, current_line);
    }

    //More complicated cases
    if(c == '/'){
        advance_input(token_value, c);
        if(c == '/'){
            advance_input(token_value, c);
            while(current_pos.first == starting_position.first){
                advance_input(token_value, c);
            }
            return create_token(token::TokenType::COMMENT, token_value, starting_position, current_pos, current_line);
        }
        if(c == '*'){
            advance_input(token_value, c);
            while(true){
                if(c == '*'){
                    advance_input(token_value, c);
                    if(c == '/'){
                        advance_input(token_value, c);
                        return create_token(token::TokenType::COMMENT, token_value, starting_position, current_pos, current_line);
                    }
                }else{
                    advance_input(token_value, c);
                }
            }
        }
        if(c == '='){
            advance_input(token_value, c);
            return create_token(token::TokenType::DivAssign, token_value, starting_position, current_pos, current_line);
        }
        return create_token(token::TokenType::Div, token_value, starting_position, current_pos, current_line);
    }
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
            return Tokenizer::TokenizingSubmethods::lex_decimal_fractional(*this, c, token_value);
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
        //This assert check no longer works because of trigraphs
        //But if you disallow trigraphs, it should always pass
        //assert(starting_position == current_pos); //Make sure we're actually lexing a single character token
        auto type = single_char_tokens.at(c);
        advance_input(token_value, c);
        return create_token(type, token_value, starting_position, current_pos, current_line);
    }

    //Other cases/not implemented yet/not parsable
    throw lexer_error::UnknownInput("Unknown input", token_value, c, starting_position);
    return create_token(token::TokenType::END, "", starting_position, current_pos, this->current_line);
}

token::Token Tokenizer::TokenizingSubmethods::lex_decimal_fractional(Tokenizer& l, char& c, std::string& token_value){
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
token::Token Tokenizer::TokenizingSubmethods::lex_hex_fractional(Tokenizer& l, char& c, std::string& token_value){
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

void Tokenizer::TokenizingSubmethods::handle_int_literal_suffix(Tokenizer& l, char& c, std::string& token_value){
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

token::Token Tokenizer::TokenizingSubmethods::lex_numeric_literals(Tokenizer& l){
    char c = l.custom_peek();
    std::string token_value = "";
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
                Tokenizer::TokenizingSubmethods::handle_int_literal_suffix(l,c,token_value);
                return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
            }else{
                //Hex floating point
                return Tokenizer::TokenizingSubmethods::lex_hex_fractional(l,c,token_value);
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
                Tokenizer::TokenizingSubmethods::handle_int_literal_suffix(l,c,token_value);
                return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
            }else{
                //Decimal float with a leading 0
                return Tokenizer::TokenizingSubmethods::lex_decimal_fractional(l,c,token_value);
            }
        }
    }
    while(std::isdigit(c)){
        l.advance_input(token_value,c);
    }
    if(c != '.' && c != 'e' && c != 'E'){
        Tokenizer::TokenizingSubmethods::handle_int_literal_suffix(l,c,token_value);
        return create_token(token::TokenType::IntegerLiteral, token_value, starting_position, l.current_pos, l.current_line);
    }else{
        //Decimal floating point
        return Tokenizer::TokenizingSubmethods::lex_decimal_fractional(l,c,token_value);
    }
}


} //namespace lexer
