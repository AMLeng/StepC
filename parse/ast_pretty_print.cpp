#include "ast.h"
#include <string>
namespace ast{

template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

void AST::print_whitespace(int depth, std::ostream& output){
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

void Program::pretty_print(int depth){
    main_method->pretty_print(depth);
}
void CompoundStmt::pretty_print(int depth){
    for(const auto& stmt : stmt_body){
        stmt -> pretty_print(depth);
    }
}
void DeclList::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "DECLARATIONS:" << std::endl;
    for(const auto& decl : decls){
        decl -> pretty_print(depth + 1);
    }
}
void WhileStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "WHILE STMT WITH CONDITION: "<<std::endl;
    control_expr->pretty_print(depth + 1);
    AST::print_whitespace(depth);
    std::cout<< "AND BODY: "<<std::endl;
    body->pretty_print(depth + 1);
}
void DoStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "DO WHILE STMT WITH CONDITION: "<<std::endl;
    control_expr->pretty_print(depth + 1);
    AST::print_whitespace(depth);
    std::cout<< "AND BODY: "<<std::endl;
    body->pretty_print(depth + 1);
}
void ForStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "FOR STMT WITH INIT: "<<std::endl;

    std::visit(overloaded{
        [depth](std::monostate) -> void{
            AST::print_whitespace(depth+1);
            std::cout<< "NONE"<<std::endl;
            },
        [depth](auto& ast_node) -> void{
            ast_node->pretty_print(depth+1);
            },
    },this->init_clause);
    AST::print_whitespace(depth);
    std::cout<< "CONTROL STMT: "<<std::endl;
    control_expr->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<< "POST EXPR: "<<std::endl;
    if(this->post_expr.has_value()){
        this->post_expr.value()->pretty_print(depth+1);
    }else{
        AST::print_whitespace(depth+1);
        std::cout<< "NONE"<<std::endl;
    }
    AST::print_whitespace(depth);
    std::cout<< "FOR BODY: "<<std::endl;
    body->pretty_print(depth+1);
}


void FunctionDef::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< type::to_string(return_type) << " FUNCTION "<<name <<":"<<std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "PARAMS: ()" << std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "BODY: " << std::endl;
    function_body->pretty_print(depth + 2);
}
void Conditional::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "TERNARY CONDITIONAL ON:"<<std::endl;
    cond->pretty_print(depth+1);
    std::cout<< "TRUE EXPR:"<<std::endl;
    true_expr->pretty_print(depth+1);
    std::cout<< "FALSE EXPR:"<<std::endl;
    false_expr->pretty_print(depth+1);
}

void NullStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "NULL"<<std::endl;
}

void IfStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "IF COND:"<<std::endl;
    if_condition->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<< "IF BODY:"<<std::endl;
    if_body->pretty_print(depth+1);
    if(this->else_body.has_value()){
        AST::print_whitespace(depth);
        std::cout<< "ELSE:"<<std::endl;
        else_body.value()->pretty_print(depth+1);
    }
}
void GotoStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "GOTO STMT WITH LABEL "<<ident_tok.value<<":"<<std::endl;
}
void LabeledStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "LABELED STMT WITH LABEL "<<ident_tok.value<<":"<<std::endl;
    stmt->pretty_print(depth+1);
}
void BreakStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "BREAK STMT"<<std::endl;
}
void ContinueStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "CONTINUE STMT"<<std::endl;
}
void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN:"<<std::endl;
    return_expr->pretty_print(depth+1);
}
void Variable::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE \""<<variable_name<<"\" OF TYPE "<<type::to_string(type)<<std::endl;
}
void VarDecl::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE DECL \""<<name<<"\" OF TYPE "<< type::to_string(type) <<std::endl;
    if(this->assignment.has_value()){
        this->assignment.value()->pretty_print(depth+1);
    }
}
void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<" OF TYPE "<< type::to_string(type) <<std::endl;
}
void Postfix::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"POSTFIX OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
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
