#include "ast.h"
#include <cassert>
#include <cctype>
#include <limits>
#include "sem_error.h"
namespace ast{
namespace{
void correct_int_type(std::string& original_type, std::string& original_value, token::Token tok){
    bool consider_signed = (original_type.find("unsigned") == std::string::npos);
    bool is_dec = (original_value.at(0) != '0');
    unsigned long long int int_value = -1;
    try{
        int_value = std::stoull(original_value, nullptr, 0);
    }catch(const std::out_of_range& e){
        throw sem_error::TypeError("Integer literal too large for unsigned long long",tok);
    }
    assert(int_value >= 0);
    original_value = std::to_string(int_value);
    if(original_type.find("long") == std::string::npos){
        if(consider_signed){
            if(int_value <= std::numeric_limits<int>::max()){
                original_type = "int";
                return;
            }
        }
        if(!consider_signed || !is_dec){
            if(int_value <= std::numeric_limits<unsigned int>::max()){
                original_type = "unsigned int";
                return;
            }
        }
    }
    if(original_type.find("long long") == std::string::npos){
        if(consider_signed){
            if(int_value <= std::numeric_limits<long int>::max()){
                original_type = "long int";
                return;
            }
        }
        if(!consider_signed || !is_dec){
            if(int_value <= std::numeric_limits<unsigned long int>::max()){
                original_type = "unsigned long int";
                return;
            }
        }
    }
    if(consider_signed){
        if(int_value <= std::numeric_limits<long long int>::max()){
            original_type = "long long int";
            return;
        }
    }
    if(!consider_signed || !is_dec){
        if(int_value <= std::numeric_limits<unsigned long long int>::max()){
            original_type = "unsigned long long int";
            return;
        }
    }
    throw sem_error::TypeError("Not valid integer literal",tok);
}
} //namespace

Constant::Constant(token::Token tok){
    literal = tok.value;
    switch(tok.type){
        case token::TokenType::IntegerLiteral:
            while(!std::isxdigit(literal.back())){
                auto c = literal.back();
                literal.pop_back();
                if(c == 'u' || c == 'U'){
                    type = "unsigned " + type;
                    continue;
                }
                if(c == 'l' || c == 'L'){
                    type = type + "long ";
                    continue;
                }
                //Unreachable
                std::cout<<"This should be unreachable"<<std::endl;
                assert(false);
            }
            type = type + "int";
            correct_int_type(type, literal, tok);
            break;
        case token::TokenType::FloatLiteral:
            switch(literal.back()){
                case 'l':
                case 'L':
                    literal.pop_back();
                    type = "long double";
                    break;
                case 'f':
                case 'F':
                    literal.pop_back();
                    type = "float";
                    break;
                default:
                    type = "double";
                    assert(std::isdigit(literal.back()));

            }
            break;
        default:
            std::cout<<"This should be unreachable"<<std::endl;
            assert(false);
    }
}
} //namespace ast
