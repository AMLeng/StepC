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

void Program::codegen(std::ostream& output){
    main_method->codegen(output);
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

void FunctionDef::codegen(std::ostream& output){
    assert(return_type == "int");
    output << "define i32 @" + name+"(){"<<std::endl;
    function_body->codegen(output);
    output << "}"<<std::endl;
}

void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN ";
    return_value->pretty_print(0);
}
void ReturnStmt::codegen(std::ostream& output){
    AST::print_whitespace(1);
    output << "ret i32 ";
    return_value->codegen(output);
    output << std::endl;;
}

void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<value<<" OF TYPE "<<type<<std::endl;
}
void Constant::codegen(std::ostream& output){
    output << value;
}

} //namespace ast
