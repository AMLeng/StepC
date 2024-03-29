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

void Program::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "PROGRAM WITH:" << std::endl;
    for(const auto& decl : decls){
        decl -> pretty_print(depth);
    }
}
void CompoundStmt::pretty_print(int depth) const{
    for(const auto& stmt : stmt_body){
        stmt -> pretty_print(depth);
    }
}
void DeclList::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    if(tag_decls.size() > 0){
        std::cout<<"DECLARING TAGS: "<<std::endl;
        for(const auto& t : tag_decls){
            t->pretty_print(depth+1);
        }
    }
    AST::print_whitespace(depth);
    std::cout<< "DECLARATIONS:" << std::endl;
    for(const auto& decl : decls){
        decl -> pretty_print(depth + 1);
    }
}
void SwitchStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "SWITCH STMT WITH CONDITION: "<<std::endl;
    control_expr->pretty_print(depth + 1);
    AST::print_whitespace(depth);
    std::cout<< "AND BODY: "<<std::endl;
    switch_body->pretty_print(depth + 1);
}
void WhileStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "WHILE STMT WITH CONDITION: "<<std::endl;
    control_expr->pretty_print(depth + 1);
    AST::print_whitespace(depth);
    std::cout<< "AND BODY: "<<std::endl;
    body->pretty_print(depth + 1);
}
void DoStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "DO WHILE STMT WITH CONDITION: "<<std::endl;
    control_expr->pretty_print(depth + 1);
    AST::print_whitespace(depth);
    std::cout<< "AND BODY: "<<std::endl;
    body->pretty_print(depth + 1);
}
void ForStmt::pretty_print(int depth) const{
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


void FunctionDef::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    if(tag_decls.size() > 0){
        std::cout<<"DECLARING TAGS: "<<std::endl;
        for(const auto& t : tag_decls){
            t->pretty_print(depth+1);
        }
    }
    FunctionDecl::pretty_print(depth);
    AST::print_whitespace(depth);
    std::cout<< "FUNCTION DEF BODY: " << std::endl;
    function_body->pretty_print(depth + 2);
}
void Conditional::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "TERNARY CONDITIONAL ON:"<<std::endl;
    cond->pretty_print(depth+1);
    std::cout<< "TRUE EXPR:"<<std::endl;
    true_expr->pretty_print(depth+1);
    std::cout<< "FALSE EXPR:"<<std::endl;
    false_expr->pretty_print(depth+1);
}

void NullStmt::pretty_print(int depth) const{
}

void IfStmt::pretty_print(int depth) const{
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
void GotoStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "GOTO STMT WITH LABEL "<<ident_tok.value<<":"<<std::endl;
}
void CaseStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "CASE STMT WITH LABEL ";
    label->pretty_print(0);
    std::cout<< ": "<<std::endl;
    stmt->pretty_print(depth+1);
}
void DefaultStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "DEFAULT STMT:"<<std::endl;
    stmt->pretty_print(depth+1);
}
void LabeledStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "LABELED STMT WITH LABEL "<<ident_tok.value<<":"<<std::endl;
    stmt->pretty_print(depth+1);
}
void BreakStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "BREAK STMT"<<std::endl;
}
void ContinueStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "CONTINUE STMT"<<std::endl;
}
void ReturnStmt::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<< "RETURN:"<<std::endl;
    if(return_expr.has_value()){
        return_expr.value()->pretty_print(depth+1);
    }else{
        AST::print_whitespace(depth+1);
        std::cout<< "\"void\""<<std::endl;
    }
}
void Variable::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE \""<<variable_name<<"\" OF TYPE "<<type::to_string(type)<<std::endl;
}
void FunctionDecl::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"FUNCTION DECL \""<<name<<"\" OF TYPE "<< type::to_string(type) <<std::endl;
}
void TypedefDecl::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"TYPEDEF "<<name<<" TO "<<type::to_string(type)<<std::endl;
}
void AmbiguousBlock::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"AMBIGUOUS BLOCK STARTING WITH TOKEN: "<<this->unparsed_tokens.front().to_string()<<std::endl;
}
void TagDecl::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"TAG "<<std::visit(type::overloaded{
        [](type::EnumType t)->std::string{return "enum "+t.tag;},
        [](auto& t)->std::string{return type::to_string(t);}
    },type)<<std::endl;
}
void EnumVarDecl::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"ENUM MEMBER DECL \""<<tok.value<<"\""<<std::endl;
    this->initializer->initializer_print(depth);
}
void VarDecl::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"VARIABLE DECL \""<<name<<"\" OF TYPE "<< type::to_string(type) <<std::endl;
    if(this->assignment.has_value()){
        this->assignment.value()->initializer_print(depth);
    }
}
void Expr::initializer_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"INITIALIZER: "<<std::endl;
    this->pretty_print(depth+1);
}
void InitializerList::initializer_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"INITIALIZER List: "<<std::endl;
    for(const auto &i : initializers){
        i->initializer_print(depth+1);
    }
}
void StrLiteral::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"STRING LITERAL "<<literal<<std::endl;
}
void Sizeof::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"SIZEOF EXPR:";
    arg->pretty_print(depth+1);
}
void Alignof::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"ALIGNOF EXPR:";
    arg->pretty_print(depth+1);
}
void Constant::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<" OF TYPE "<< type::to_string(type) <<std::endl;
}
void FuncCall::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"FUNCTION CALL OF \""<< tok.value <<"\" ON ARGS"<<std::endl;
    for(const auto& arg : args){
        arg->pretty_print(depth+1);
    }
}
void MemberAccess::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"ARRAY ACCESS OF MEMBER "<<index<<" IN"<<std::endl;
    arg->pretty_print(depth+1);
}
void ArrayAccess::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"ARRAY ACCESS OF "<<std::endl;
    arg->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<<"INDEX "<<std::endl;
    index->pretty_print(depth+1);
}
void Postfix::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"POSTFIX OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}
void UnaryOp::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"UNARY OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}

void BinaryOp::pretty_print(int depth) const{
    AST::print_whitespace(depth);
    std::cout<<"BINARY OP "<< tok.type <<" WITH LEFT ARG"<<std::endl;
    left->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<<"AND RIGHT ARG"<<std::endl;
    right->pretty_print(depth+1);
}

}//namespace ast
