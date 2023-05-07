#include "ast.h"
#include "sem_error.h"
#include "type.h"
#include <string>
#include <cassert>
namespace ast{
void AST::print_whitespace(int depth, std::ostream& output){
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

void Program::pretty_print(int depth){
    main_method->pretty_print(depth);
}

std::unique_ptr<value::Value> Program::codegen(std::ostream& output, context::Context& c){
    main_method->codegen(output, c);
    return nullptr;
}

void FunctionDef::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< return_type<< " FUNCTION "<<name <<":"<<std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "PARAMS: ()" << std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "BODY: " << std::endl;
    function_body -> pretty_print(depth+2);
}

std::unique_ptr<value::Value> FunctionDef::codegen(std::ostream& output, context::Context& c){
    assert(return_type == "int");
    AST::print_whitespace(c.current_depth, output);
    output << "define i32 @" + name+"(){"<<std::endl;
    c.current_depth++;
    function_body->codegen(output, c);
    c.current_depth--;
    AST::print_whitespace(c.current_depth, output);
    output << "}"<<std::endl;
    return std::make_unique<value::Value>("@"+name);
}

void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN:"<<std::endl;
    return_expr->pretty_print(depth+1);
}
std::unique_ptr<value::Value> ReturnStmt::codegen(std::ostream& output, context::Context& c){
    auto return_value = return_expr->codegen(output, c);
    AST::print_whitespace(c.current_depth, output);
    //Since right now we only return from main
    output << "ret "+type::ir_type(type::from_str("int"))+" "+ return_value->get_value() << std::endl;
    return nullptr;
}


void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<" of type "<< to_string(type) <<std::endl;
}

std::unique_ptr<value::Value> Constant::codegen(std::ostream& output, context::Context& c){
    return std::make_unique<value::Value>(this->literal);
}

void UnaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"UNARY OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}

std::unique_ptr<value::Value> UnaryOp::codegen(std::ostream& output, context::Context& c){
    auto inner_exp_register = arg->codegen(output, c);
    switch(tok.type){
        case token::TokenType::Plus:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = add i32 0, " <<inner_exp_register->get_value() <<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::Minus:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = sub i32 0, " <<inner_exp_register->get_value() <<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::BitwiseNot:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = xor i32 -1, " <<inner_exp_register->get_value() <<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::Not:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = icmp eq i32 0, " <<inner_exp_register->get_value() <<std::endl;
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = zext i1 "<< c.prev_temp(1) <<" to i32"<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        default:
            assert(false && "Operator Not Implemented");
    }
}

void BinaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"BINARY OP "<< tok.type <<" WITH LEFT ARG"<<std::endl;
    left->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<<"AND RIGHT ARG"<<std::endl;
    right->pretty_print(depth+1);
}

std::unique_ptr<value::Value> BinaryOp::codegen(std::ostream& output, context::Context& c){
    auto left_register = left->codegen(output, c);
    auto right_register = right->codegen(output, c);
    switch(tok.type){
        case token::TokenType::Minus:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = sub i32 " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::Plus:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = add i32 " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::Mult:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = mul i32 " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        case token::TokenType::Div:
            AST::print_whitespace(c.current_depth, output);
            output << c.new_temp()<<" = sdiv i32 " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0));
        default:
            assert(false && "Unknown binary op during codegen");
    }
}

} //namespace ast
