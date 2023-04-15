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
    output << "define i32 @" + name+"(){"<<std::endl;
    function_body->codegen(output, c);
    output << "}"<<std::endl;
    return std::make_unique<value::Value>("@"+name);
}

void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN ";
    return_expr->pretty_print(0);
}
std::unique_ptr<value::Value> ReturnStmt::codegen(std::ostream& output, context::Context& c){
    AST::print_whitespace(1);
    auto return_value = return_expr->codegen(output, c);
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
    std::cout<<"This should be unreachable"<<std::endl;
    assert(false);
}
} //namespace ast
