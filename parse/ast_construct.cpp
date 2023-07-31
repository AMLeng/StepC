#include "ast.h"
#include "type.h"
#include "parse_error.h"
#include "sem_error.h"
#include <cassert>
#include <limits>
#include <sstream>
namespace ast{
namespace{
void finish_literal_int_parse(type::CType& original_type, std::string& original_value, token::Token tok){
    type::IType int_type = type::get<type::IType>(original_type);

    //Consider unsigned if either originally unsigned or is not decimal literal
    bool consider_unsigned = type::is_unsigned_int(original_type) || (original_value.at(0) == '0');
    unsigned long long int value = -1;
    try{
        value = std::stoull(original_value, nullptr, 0);
    }catch(const std::out_of_range& e){
        throw sem_error::TypeError("Integer literal too large for unsigned long long",tok);
    }
    assert(value >= 0);
    original_value = std::to_string(value);
    do{
        if(type::can_represent(int_type, value)){
            original_type = int_type;
            return;
        }
        if(consider_unsigned && type::can_represent(type::to_unsigned(int_type),value)){
            original_type = type::to_unsigned(int_type);
            return;
        }
    }while(type::promote_one_rank(int_type));
    throw sem_error::TypeError("Unsigned long long required to hold signed integer literal",tok);
}
std::map<char, char> escape_chars = {{
    {'a','\a'},{'b','\b'},{'f','\f'},{'n','\n'},
    {'r','\r'},{'t','\t'},{'v','\v'},{'\\','\\'},
    {'\'','\''},{'\"','\"'},{'\?','\?'}
}};

} //namespace

AST::~AST(){}
Initializer::~Initializer(){}
Decl::~Decl(){}
Stmt::~Stmt(){}
BlockItem::~BlockItem(){}
Expr::~Expr(){}
ExtDecl::~ExtDecl(){}

StrLiteral::StrLiteral(std::vector<token::Token> toks) : Expr(toks.front()){
    auto ss = std::stringstream{};
    char back;
    for(const auto& tok : toks){
        auto string = tok.value.substr(1,tok.value.size()-2);
        back = string.back();
        for(int i=0; i<string.size(); i++){
            if(string.at(i) != '\\'){
                ss << string.at(i);
            }else{
                i++;
                if(string.at(i) == '0'){
                    //Hit an early null character
                    ss << '\0';
                    this->literal = ss.str();
                    return;
                }
                if(escape_chars.find(string.at(i)) == escape_chars.end()){
                    throw parse_error::ParseError("Unknown escape sequence",tok);
                }
                ss << escape_chars.at(string.at(i));
            }
        }
    }
    if(back != '\0'){
        ss << '\0';
    }
    this->literal = ss.str();

}

Constant::Constant(const token::Token& tok) : Expr(tok){
    literal = tok.value;
    std::string type_str = "";
    switch(tok.type){
        case token::TokenType::IntegerLiteral:
            while(!std::isxdigit(literal.back())){
                auto c = literal.back();
                literal.pop_back();
                if(c == 'u' || c == 'U'){
                    type_str = "unsigned " + type_str;
                    continue;
                }
                if(c == 'l' || c == 'L'){
                    type_str = type_str + "long ";
                    continue;
                }
                //Unreachable
                assert(false && "This should be unreachable");
            }
            type_str = type_str + "int";
            type = type::from_str(type_str);
            finish_literal_int_parse(type, literal, tok);
            break;
        case token::TokenType::FloatLiteral:
            switch(literal.back()){
                case 'l':
                case 'L':
                    literal.pop_back();
                    type = type::from_str("long double");
                    break;
                case 'f':
                case 'F':
                    literal.pop_back();
                    type = type::from_str("float");
                    break;
                default:
                    type = type::from_str("double");
                    assert(std::isdigit(literal.back()));

            }
            //literal = type::ir_literal(literal, type::get<type::BasicType>(type));
            break;
        default:
            assert(false && "Unknown literal type");
    }
}
} //namespace ast
