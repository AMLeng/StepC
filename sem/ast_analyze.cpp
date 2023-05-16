#include "ast.h"
#include "type.h"
#include "sem_error.h"
namespace ast{

void VarDecl::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Add symbol to symbol table, check that not already present
    try{
        st->add_symbol(this->name,this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    //If we have a declaration attached
    if(this->assignment.has_value()){
        this->assignment.value()->analyze(st);
    }
}
void Variable::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Check that the variable name actually exists in a symbol table
    if(!st->has_symbol(this->variable_name)){
        throw sem_error::STError("Variable not found in symbol table",this->tok);
    }
    this->type=st->symbol_type(this->variable_name);
}
void Conditional::analyze(symbol::STable* st){
    this->analyzed = true;
    cond->analyze(st);
    if(!type::is_scalar(this->cond->type)){
        throw sem_error::TypeError("Condition of scalar type required for ternary conditional",this->cond->tok);
    }
    true_expr->analyze(st);
    false_expr->analyze(st);
    if(!type::is_arith(this->true_expr->type) || !type::is_arith(this->false_expr->type)){
        throw sem_error::UnknownError("Ternary conditional returning non arithmetic type no yet implemented",this->tok);
    }
    this->type = type::usual_arithmetic_conversions(this->true_expr->type, this->false_expr->type);
}
void UnaryOp::analyze(symbol::STable* st) {
    this->analyzed = true;
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
    this->analyzed = true;
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
void NullStmt::analyze(symbol::STable* st){
}
void Constant::analyze(symbol::STable* st){
    this->analyzed = true;
}
void IfStmt::analyze(symbol::STable* st){
    this->if_condition->analyze(st);
    this->if_body->analyze(st);
    if(this->else_body.has_value()){
        this->else_body.value()->analyze(st);
    }
}
void ReturnStmt::analyze(symbol::STable* st){
    return_expr->analyze(st);
}
void Program::analyze(symbol::STable* st) {
    auto main_table = st->new_child();
    main_method->analyze(main_table);
}
void CompoundStmt::analyze(symbol::STable* st){
    auto stmt_table = st->new_child();
    for(auto& stmt : stmt_body){
        stmt->analyze(stmt_table);
    }
}
void FunctionDef::analyze(symbol::STable* st) {
    if(this->name == "main"){
        if(function_body->stmt_body.size() == 0 || !dynamic_cast<ReturnStmt*>(function_body->stmt_body.back().get())){
            auto fake_token = token::Token{token::TokenType::IntegerLiteral, "0",{-1,-1,-1,-1},"COMPILER GENERATED TOKEN, SOURCE LINE NOT AVAILABLE"};
            std::unique_ptr<Expr> ret_expr = std::make_unique<Constant>(fake_token);
            function_body->stmt_body.push_back(std::make_unique<ReturnStmt>(std::move(ret_expr)));
        }
    }
    function_body->analyze(st);
}
} //namespace ast