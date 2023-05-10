#include "ast.h"
#include <string>
namespace ast{
void AST::print_whitespace(int depth, std::ostream& output){
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

void Program::pretty_print(int depth){
    main_method->pretty_print(depth);
}

void FunctionDef::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< type::to_string(return_type) << " FUNCTION "<<name <<":"<<std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "PARAMS: ()" << std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "BODY: " << std::endl;
    for(const auto& stmt : function_body){
        stmt -> pretty_print(depth+2);
    }
}

void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN:"<<std::endl;
    return_expr->pretty_print(depth+1);
}
void Variable::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE \""<<variable_name<<"\""<<std::endl;
}
void Assign::pretty_print(int depth){
    left->pretty_print(depth);
    AST::print_whitespace(depth+ 1);
    std::cout<<" ASSIGNED TO "<<std::endl;
    right->pretty_print(depth +2);
}
void VarDecl::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE DECL "<<name<<" of type "<< type::to_string(type) <<std::endl;
}
void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<" of type "<< type::to_string(type) <<std::endl;
}
void UnaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"UNARY OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}

void BinaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"BINARY OP "<< tok.type <<" WITH LEFT ARG"<<std::endl;
    left->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<<"AND RIGHT ARG"<<std::endl;
    right->pretty_print(depth+1);
}

}//namespace ast
