#ifndef _TOKEN_
#define _TOKEN_
#include "location.h"
#include <string>
#include <sstream>
#include <ostream>
namespace token{
enum class TokenType{
    LBrace, RBrace, LParen, RParen, LBrack, RBrack, Semicolon, Period,
    Keyword, Identifier, IntegerLiteral, FloatLiteral,
    Not, Minus, Plus, Mult, Div, Mod,
    Assign, PlusAssign, MinusAssign, DivAssign, MultAssign,ModAssign,
    LSAssign,RSAssign,BAAssign, BOAssign, BXAssign,
    Colon, Question, And, Or,
    Equal,NEqual,Greater,Less,LEq,GEq,
    BitwiseNot,BitwiseAnd,BitwiseOr,BitwiseXor, LShift,RShift,
    Comma,Plusplus, Minusminus,
    END, COMMENT
};

struct Token{

    TokenType type;
    std::string value;
    location::Location loc;
    std::string sourceline;
    static Token make_end_token(std::pair<int, int> position){
        location::Location loc = {position.first, position.second, position.first, position.second};
        return Token{TokenType::END, "", loc, ""};
    }
    std::string to_string() const{
        auto ss = std::stringstream{};
        ss<< "token: " << value << std::endl;
        ss<<"At line " <<loc.start_line <<" and column "<<loc.start_col <<std::endl;
        if(loc.start_line == loc.end_line){
            auto line = std::to_string(loc.start_line);
            ss << line <<" |";
            ss<<sourceline<<std::endl;

            for(int i=0; i<line.size(); i++){
                ss << " ";
            }
            ss << " |";
            for(int i=1; i<loc.start_col; i++){
                ss << " ";
            }
            for(int i=loc.start_col; i<loc.end_col; i++){
                ss << "^";
            }
            ss<<std::endl;
        }
        return ss.str();
    }
};

inline std::string string_name(TokenType type){
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
        case TokenType::Comma:
            return "comma";
        case TokenType::Plusplus:
            return "plus plus";
        case TokenType::Minusminus:
            return "minusminus";
        case TokenType:: BitwiseNot:
            return "bitwise not";
        case TokenType:: BitwiseAnd:
            return "bitwise and";
        case TokenType:: BitwiseOr:
            return "bitwise or";
        case TokenType:: BitwiseXor:
            return "bitwise xor";
        case TokenType::LShift:
            return "left bitshift";
        case TokenType::RShift:
            return "right bitshift";
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
        case TokenType::Mod:
            return "mod";
        case TokenType::And:
            return "logical and";
        case TokenType::Or:
            return "logical or";
        case TokenType::Equal:
            return "equal";
        case TokenType::NEqual:
            return "not equal";
        case TokenType::Less:
            return "less than";
        case TokenType::Greater:
            return "greater than";
        case TokenType::LEq:
            return "less than or equal to";
        case TokenType::GEq:
            return "greater than or equal to";
        case TokenType::PlusAssign:
            return "plus assign compound operator";
        case TokenType::MinusAssign:
            return "minus assign compound operator";
        case TokenType::DivAssign:
            return "division assign compound operator";
        case TokenType::MultAssign:
            return "multiplication assign compound operator";
        case TokenType::ModAssign:
            return "mod assign compound operator";
        case TokenType::LSAssign:
            return "left shift assign compound operator";
        case TokenType::RSAssign:
            return "right shift assign compound operator";
        case TokenType::BAAssign:
            return "binary and assign compound operator";
        case TokenType::BOAssign:
            return "binary or assign compound operator";
        case TokenType::BXAssign:
            return "binary xor assign compound operator";
        case TokenType::Assign:
            return "assignment operator";
        case TokenType::Keyword:
            return "keyword";
        case TokenType::Identifier:
            return "identifier";
        case TokenType::IntegerLiteral:
            return "integer literal";
        case TokenType::Colon:
            return "colon ':'";
        case TokenType::Question:
            return "question mark '?'";
        case TokenType::COMMENT:
            return "comment";
        case TokenType::END:
            return "end of input stream";
    }
    //Annotation or g++ complains
    __builtin_unreachable();
    assert(false);
}
//Definitions for helper methods
inline bool matches_type(const token::Token& tok){
    return false;
}

template <typename... TokTypes>
bool matches_type(const token::Token& tok, token::TokenType t, TokTypes... types){
    return tok.type == t || matches_type(tok, types...);
}

inline bool matches_keyword(const token::Token& tok){
    assert(tok.type == token::TokenType::Keyword);
    return false;
}

template<typename... Ts>
bool matches_keyword(const token::Token& tok, std::string_view keyword, Ts... ts){
    return tok.value == keyword || matches_keyword(tok, ts...);
}
}//namespace token

namespace std{
inline ostream& operator<<(ostream& out, token::TokenType type){
    out << token::string_name(type);
    return out;
}
}
#endif
