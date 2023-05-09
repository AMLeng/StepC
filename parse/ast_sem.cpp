#include "ast.h"
#include <cctype>
#include <cassert>
#include <limits>
#include "type.h"
#include "sem_error.h"
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
namespace ast{
namespace{
void finish_literal_int_parse(type::BasicType& original_type, std::string& original_value, token::Token tok){
    assert(type::is_int(original_type));
    type::IType int_type = std::get<type::IType>(original_type);

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
            original_type = type::make_basic(int_type);
            return;
        }
        if(consider_unsigned && type::can_represent(type::to_unsigned(int_type),value)){
            original_type = type::make_basic(type::to_unsigned(int_type));
            return;
        }
    }while(type::promote_one_rank(int_type));
    throw sem_error::TypeError("Unsigned long long required to hold signed integer literal",tok);
}

std::string float_to_hex(const std::string& literal_value, type::BasicType type){
    //For now we "cheat" in two ways
    //First, we use the C library functions
    //Second, we make long doubles the same as doubles
    //Together, this lets us avoid implementing arbitrary precision floats
    //And avoids dealing with target-dependent long doubles
    //Since llvm IR for long doubles is not target independent
    assert(type::is_float(type));
    std::stringstream stream;
    double value = std::stod(literal_value);
    std::uint64_t num = 0;
    static_assert(sizeof(value) == 8);
    static_assert(std::numeric_limits<double>::is_iec559);
    std::memcpy(&num, &value, 8);
    if(std::get<type::FType>(type) == type::FType::Float){
        std::uint64_t exp = 0;
        std::memcpy(&exp, &value, 8);
        //Clear out exponent from num
        num <<= 12; 
        num >>= 12;
        //And clear significand from exp
        exp >>= 52; 
        int denormal_precision_loss = 0x381 - exp; //0x380 is equivalent to all 0s for a float
        if(denormal_precision_loss < 0){
            denormal_precision_loss = 0;
        }
        exp <<= 52; 
        //Remove precision from num to make it as precise as a float
        unsigned int sig_figs_lost = (52-23 + denormal_precision_loss);
        std::uint64_t bits_lost = (num << (64-sig_figs_lost)) >> (64 - sig_figs_lost);
        std::uint64_t rounding_mask = 1 << (sig_figs_lost - 1);
        num >>= sig_figs_lost;
        if((bits_lost & rounding_mask) != 0){
            if((bits_lost & (~rounding_mask)) != 0
                || ((num & 1u) == 1)
                ){
                //Either above 1/2 ULP, or we round to even
                num++;
            }
        }
        num <<= sig_figs_lost;
        num += exp;
    }
    stream << "0x" << std::hex << std::uppercase<<num;
    return stream.str();
}
} //namespace

AST::~AST(){}
Decl::~Decl(){}
Stmt::~Stmt(){}
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
            literal = float_to_hex(literal, type);
            break;
        default:
            assert(false && "Unknown literal type");
    }
}

UnaryOp::UnaryOp(token::Token op, std::unique_ptr<Expr> exp) : 
    Expr(op), arg(std::move(exp)) {
    //Typechecking
    switch(op.type){
        case token::TokenType::Plus:
        case token::TokenType::Minus:
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",this->arg->tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
        case token::TokenType::Not:
            if(!type::is_scalar(this->arg->type)){
                throw sem_error::TypeError("Operand of scalar type required",this->arg->tok);
            }
            this->type = type::from_str("int");
            break;
        case token::TokenType::BitwiseNot:
            if(!type::is_int(this->arg->type)){
                throw sem_error::TypeError("Operand of integer type required", this->arg->tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
        default:
            assert(false && "Unknown unary operator type");
    }
}
BinaryOp::BinaryOp(token::Token op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : 
    Expr(op), left(std::move(left)), right(std::move(right)) {
    switch(op.type){
        case token::TokenType::Plus:
        case token::TokenType::Minus:
        case token::TokenType::Mult:
        case token::TokenType::Div:
            if(!type::is_arith(this->left->type) || !type::is_arith(this->right->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            this->type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            break;
        default:
            assert(false && "Unknown binary operator type");
    }
}
} //namespace ast
