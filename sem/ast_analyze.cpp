#include "ast.h"
#include "type.h"
#include "sem_error.h"
namespace ast{

void VarDecl::analyze(symbol::STable* st) {
    //Add symbol to symbol table, check that not already present
    try{
        st->add_symbol(this->name,this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
}
void Variable::analyze(symbol::STable* st) {
    //Check that the variable name actually exists in a symbol table
    if(!st->has_symbol(this->variable_name)){
        throw sem_error::STError("Variable not found in symbol table",this->tok);
    }
}
void UnaryOp::analyze(symbol::STable* st) {
    this->arg->analyze(st);
    //Typechecking
    switch(this->tok.type){
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
void BinaryOp::analyze(symbol::STable* st){
    this->left->analyze(st);
    this->right->analyze(st);
    switch(this->tok.type){
        case token::TokenType::Assign:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            //All basic types are interconvertable, modulo some potential for UB
            this->type = this->left->type;
            break;
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
//Methods that just recurse
void ReturnStmt::analyze(symbol::STable* st){
    return_expr->analyze(st);
}
void Program::analyze(symbol::STable* st) {
    auto main_table = st->new_child();
    main_method->analyze(main_table);
}
void FunctionDef::analyze(symbol::STable* st) {
    for(auto& stmt : function_body){
        stmt->analyze(st);
    }
}
} //namespace ast
