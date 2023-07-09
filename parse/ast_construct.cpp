#include "ast.h"
#include "type.h"
#include "sem_error.h"
#include <cassert>
#include <limits>
namespace ast{
namespace{
void finish_literal_int_parse(type::CType& original_type, std::string& original_value, token::Token tok){
    type::IType int_type = std::get<type::IType>(std::get<type::BasicType>(original_type));

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

} //namespace

AST::~AST(){}
Initializer::~Initializer(){}
Decl::~Decl(){}
Stmt::~Stmt(){}
BlockItem::~BlockItem(){}
Expr::~Expr(){}

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
            //literal = type::ir_literal(literal, std::get<type::BasicType>(type));
            break;
        default:
            assert(false && "Unknown literal type");
    }
}
} //namespace ast
