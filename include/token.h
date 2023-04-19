#ifndef _TOKEN_
#define _TOKEN_
#include "location.h"
namespace token{
enum class TokenType{
    LBrace, RBrace, LParen, RParen, LBrack, RBrack, Semicolon, Period,
    Keyword, Identifier, IntegerLiteral, FloatLiteral,
    BitwiseNot, Not, Minus, Plus, Mult, Div,
    END, COMMENT
};

struct Token{

    TokenType type;
    std::string value;
    location::Location loc;
    static Token make_end_token(std::pair<int, int> position){
        location::Location loc = {position.first, position.second, position.first, position.second};
        return Token{TokenType::END, "", loc};
    }
};

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
        case TokenType:: BitwiseNot:
            return "bitwise not";
        case TokenType::Not:
            return "logical not";
        case TokenType:: Minus:
            return "minus";
        case TokenType:: Plus:
            return "plus";
        case TokenType::Mult:
            return "multiplication";
        case TokenType::Div:
            return "division";
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

}//namespace token
#endif
