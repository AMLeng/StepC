#include "ast.h"
#include <cassert>
namespace ast{
void AST::print_whitespace(int depth){
    for(int i=0; i<depth; i++){
        std::cout << "  ";
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
    AST::print_whitespace(c.current_depth);
    output << "define i32 @" + name+"(){"<<std::endl;
    c.current_depth++;
    function_body->codegen(output, c);
    c.current_depth--;
    AST::print_whitespace(c.current_depth);
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
    AST::print_whitespace(c.current_depth);
    output << "ret i32 " + return_value->get_value() << std::endl;
    return nullptr;
}

void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<std::endl;
}

std::unique_ptr<value::Value> Constant::codegen(std::ostream& output, context::Context& c){
    return std::make_unique<value::Value>(literal);
}

void UnaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"UNARY OP "<<op<<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}

std::unique_ptr<value::Value> UnaryOp::codegen(std::ostream& output, context::Context& c){
    auto inner_exp_register = arg->codegen(output, c);
    if(op == "-"){
        AST::print_whitespace(c.current_depth);
        output << c.new_temp()<<" = sub i32 0, " <<inner_exp_register->get_value() <<std::endl;
        return std::make_unique<value::Value>(c.prev_temp(0));
    }
    if(op == "~"){
        AST::print_whitespace(c.current_depth);
        output << c.new_temp()<<" = xor i32 -1, " <<inner_exp_register->get_value() <<std::endl;
        return std::make_unique<value::Value>(c.prev_temp(0));
    }
    if(op == "!"){
        AST::print_whitespace(c.current_depth);
        output << c.new_temp()<<" = icmp eq i32 0, " <<inner_exp_register->get_value() <<std::endl;
        AST::print_whitespace(c.current_depth);
        output << c.new_temp()<<" = zext i1 "<< c.prev_temp(1) <<" to i32"<<std::endl;
        return std::make_unique<value::Value>(c.prev_temp(0));
    }
    std::cout<<"This should be unreachable"<<std::endl;
    assert(false);
}
} //namespace ast
