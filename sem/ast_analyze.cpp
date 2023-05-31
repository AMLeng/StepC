#include "ast.h"
#include "type.h"
#include "sem_error.h"
namespace ast{

template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;
void VarDecl::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Add symbol to symbol table, check that not already present
    try{
        st->add_symbol(this->name,this->type, this->assignment.has_value());
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    //If we have a declaration attached
    if(this->assignment.has_value()){
        this->assignment.value()->analyze(st);
        if(!st->in_function()){
            //If not in function, is global and needs to be
            if(!dynamic_cast<ast::Constant*>(this->assignment.value()->right.get())){
                throw sem_error::FlowError("Global variable def must be constant",this->assignment.value()->tok);
            }
        }
    }
}
void Variable::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Check that the variable name actually exists in a symbol table
    if(!st->has_symbol(this->variable_name)){
        throw sem_error::STError("Variable not found in symbol table",this->tok);
    }
    auto type_in_table = st->symbol_type(this->variable_name);
    if(!std::holds_alternative<type::BasicType>(type_in_table)){
        throw sem_error::STError("Symbol table entry not of basic type",this->tok);
    }
    this->type = type_in_table;
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
void FuncCall::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Check that the function actually exists in a symbol table
    if(!st->has_symbol(this->func_name)){
        throw sem_error::STError("Function not found in symbol table",this->tok);
    }
    auto arg_types = std::vector<type::CType>{};
    for(auto& expr : args){
        expr->analyze(st);
        arg_types.push_back(expr->type);
    }
    if(arg_types.size() == 0){
        arg_types.push_back(type::CType());
    }
    try{
        auto f_type = std::get<type::DerivedType>(st->symbol_type(this->func_name)).get<type::FuncType>();
        if(!f_type.params_match(arg_types)){
            throw sem_error::TypeError("Cannot call function of type "+type::to_string(f_type)+" on types of provided arguments",this->tok);
        }
        this->type = f_type.return_type();
    }catch(std::runtime_error& e){ //Won't catch the STError
        throw sem_error::STError("Function call with identifier not referring to a function",this->tok);
    }
}
void Postfix::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    //Typechecking
    switch(this->tok.type){
        case token::TokenType::Plusplus:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->arg.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required as argument of increment",tok);
            }
            }
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
        case token::TokenType::Minusminus:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->arg.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required as argument of decrement",tok);
            }
            }
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
        default:
            assert(false && "Unknown postfix operator type");
            break;
    }
}
void UnaryOp::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    //Typechecking
    switch(this->tok.type){
        case token::TokenType::Plusplus:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->arg.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required as argument of increment",tok);
            }
            }
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
        case token::TokenType::Minusminus:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->arg.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required as argument of decrement",tok);
            }
            }
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            break;
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
        case token::TokenType::PlusAssign:
        case token::TokenType::MinusAssign:
        case token::TokenType::DivAssign:
        case token::TokenType::MultAssign:
            if(!type::is_arith(this->left->type) || !type::is_arith(this->right->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            {
            auto convert_type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = convert_type;
            this->new_right_type = convert_type;
            }
            this->type = this->left->type;
            break;
        case token::TokenType::ModAssign:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            {
            auto convert_type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = convert_type;
            this->new_right_type = convert_type;
            }
            this->type = this->left->type;
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            break;
        case token::TokenType::BAAssign:
        case token::TokenType::BOAssign:
        case token::TokenType::BXAssign:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            {
            auto convert_type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = convert_type;
            this->new_right_type = convert_type;
            }
            this->type = this->left->type;
            break;
        case token::TokenType::LSAssign:
        case token::TokenType::RSAssign:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            this->new_left_type = type::integer_promotions(this->left->type);
            this->new_right_type = type::integer_promotions(this->right->type);
            this->type = this->left->type;
            break;
        case token::TokenType::Assign:
            {
            auto lval = dynamic_cast<ast::LValue*>(this->left.get());
            if(!lval){
                throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
            }
            }
            //All basic types are interconvertable, modulo some potential for UB
            this->new_right_type = this->right->type;
            this->new_left_type = this->left->type;
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
            this->new_left_type = this->type;
            this->new_right_type = this->type;
            break;
        case token::TokenType::Mod:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            this->type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = this->type;
            this->new_right_type = this->type;
            break;
        case token::TokenType::And:
        case token::TokenType::Or:
            if(!type::is_scalar(this->left->type) || !type::is_scalar(this->right->type)){
                throw sem_error::TypeError("Operand of scalar type required",tok);
            }
            this->new_right_type = this->right->type;
            this->new_left_type = this->left->type;
            this->type = type::from_str("int");
            break;
        case token::TokenType::Equal:
        case token::TokenType::NEqual:
            //Check that in fact real type
        case token::TokenType::Less:
        case token::TokenType::Greater:
        case token::TokenType::LEq:
        case token::TokenType::GEq:
            if(!type::is_arith(this->left->type) || !type::is_arith(this->right->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",tok);
            }
            {
            auto convert_type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = convert_type;
            this->new_right_type = convert_type;
            }
            this->type = type::from_str("int");
            break;
        case token::TokenType::BitwiseAnd:
        case token::TokenType::BitwiseOr:
        case token::TokenType::BitwiseXor:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            this->type = type::usual_arithmetic_conversions(this->left->type, this->right->type);
            this->new_left_type = this->type;
            this->new_right_type = this->type;
            break;
        case token::TokenType::LShift:
        case token::TokenType::RShift:
            if(!type::is_int(this->left->type) || !type::is_int(this->right->type)){
                throw sem_error::TypeError("Operand of integer type required",tok);
            }
            this->new_left_type = type::integer_promotions(this->left->type);
            this->new_right_type = type::integer_promotions(this->right->type);
            this->type = this->new_left_type;
            break;
        case token::TokenType::Comma:
            this->new_right_type = this->right->type;
            this->new_left_type = this->left->type;
            this->type = this->left->type;
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
    auto ret_type = type::CType(type::VoidType());
    if(return_expr.has_value()){
        return_expr.value()->analyze(st);
        if(type::is_type<type::VoidType>(return_expr.value()->type)){
            throw sem_error::TypeError("Cannot have expression with void type in return statement",this->return_expr.value()->tok);
        }
        ret_type = this->return_expr.value()->type;
    }
    if(!type::can_convert(ret_type,st->return_type())){
        throw sem_error::TypeError("Invalid return type",this->return_expr.value()->tok);
    }
}
void ContinueStmt::analyze(symbol::STable* st){
    if(!st->in_loop){
        throw sem_error::FlowError("Continue statement outside of loop",this->tok);
    }
}
void GotoStmt::analyze(symbol::STable* st){
    try{
        st->require_label(ident_tok);
    }catch(std::runtime_error& e){
        throw sem_error::FlowError("Cannot have goto outside function",this->ident_tok);
    }
    //Matched with a call at function end
}
void LabeledStmt::analyze(symbol::STable* st){
    stmt->analyze(st);
    try{
        st->add_label(ident_tok.value);
    }catch(std::runtime_error& e){
        throw sem_error::STError("Duplicate label name within the same function",this->ident_tok);
    }
}
void BreakStmt::analyze(symbol::STable* st){
    if(!st->in_loop && !st->in_switch()){
        throw sem_error::FlowError("Break statement outside of loop or switch",this->tok);
    }
}
void Program::analyze(symbol::STable* st) {
    for(auto& decl : decls){
        decl->analyze(st);
    }
}
void ForStmt::analyze(symbol::STable* st){
    auto stmt_table = st->new_child();

    std::visit(overloaded{
        [](std::monostate){/*Do nothing*/},
        [stmt_table](auto& ast_node){
            ast_node->analyze(stmt_table);
            }
    },this->init_clause);
    control_expr->analyze(stmt_table);
    if(!type::is_scalar(this->control_expr->type)){
        throw sem_error::TypeError("Condition of scalar type required in for statement control expression",this->control_expr->tok);
    }
    if(this->post_expr.has_value()){
        this->post_expr.value()->analyze(stmt_table);
    }
    stmt_table->in_loop = true;
    this->body->analyze(stmt_table);
}
void CaseStmt::analyze(symbol::STable* st){
    if(!st->in_switch()){
        throw sem_error::FlowError("Case statement outside of switch",this->tok);
    }
    this->label->analyze(st);
    if(!type::is_int(label->type)){
        throw sem_error::TypeError("Case label must have integer type",this->tok);
    }
    unsigned long long int case_val = 42ull;
    try{
        case_val = std::stoull(this->label->literal);
    }catch(std::runtime_error& e){
        throw sem_error::FlowError("Invalid label value for case",this->label->tok);
    }
    try{
        st->add_case(case_val);
    }catch(std::runtime_error& e){
        throw sem_error::STError("Duplicate case statement in switch",this->label->tok);
    }
    this->stmt->analyze(st);
}
void DefaultStmt::analyze(symbol::STable* st){
    if(!st->in_switch()){
        throw sem_error::FlowError("Case statement outside of switch",this->tok);
    }
    try{
        st->add_case(std::nullopt);
    }catch(std::runtime_error& e){
        throw sem_error::STError("Duplicate default statement in switch",this->tok);
    }
    this->stmt->analyze(st);
}
void SwitchStmt::analyze(symbol::STable* st){
    control_expr->analyze(st);
    if(!type::is_int(this->control_expr->type)){
        throw sem_error::TypeError("Condition of integer type required in for switch control expression",this->control_expr->tok);
    }
    this->control_type = type::integer_promotions(this->control_expr->type);
    auto stmt_table = st->new_switch_scope_child();
    switch_body->analyze(stmt_table);
    case_table = stmt_table->transfer_switch_table();
}
void WhileStmt::analyze(symbol::STable* st){
    control_expr->analyze(st);
    if(!type::is_scalar(this->control_expr->type)){
        throw sem_error::TypeError("Condition of scalar type required in for statement control expression",this->control_expr->tok);
    }
    auto stmt_table = st->new_child();
    stmt_table->in_loop = true;
    body->analyze(stmt_table);
}
void DoStmt::analyze(symbol::STable* st){
    control_expr->analyze(st);
    if(!type::is_scalar(this->control_expr->type)){
        throw sem_error::TypeError("Condition of scalar type required in for statement control expression",this->control_expr->tok);
    }
    auto stmt_table = st->new_child();
    stmt_table->in_loop = true;
    body->analyze(stmt_table);
}
void CompoundStmt::analyze(symbol::STable* st){
    auto stmt_table = st->new_child();
    for(auto& stmt : stmt_body){
        stmt->analyze(stmt_table);
    }
}
void DeclList::analyze(symbol::STable* st){
    this->analyzed = true;
    for(auto& decl : decls){
        decl->analyze(st);
    }
}
void FunctionDecl::analyze(symbol::STable* st){
    this->analyzed = true;
    if(st->in_function()){
        throw sem_error::FlowError("Function declaration inside function",this->tok);
    }
    //Add to global symbol table
    try{
        st->add_symbol(this->name,this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
}
void FunctionDef::analyze(symbol::STable* st) {
    this->analyzed = true;
    auto f_type = std::get<type::DerivedType>(this->type).get<type::FuncType>();
    if(st->in_function()){
        throw sem_error::FlowError("Function definition inside function",this->tok);
    }
    try{
        st->add_symbol(this->name,this->type, true);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    if(this->tok.value == "main"){
        if(f_type.return_type() != type::CType(type::IType::Int)){
            throw sem_error::TypeError("Main method must return int",this->tok);
        }
        if(function_body->stmt_body.size() == 0 || !dynamic_cast<ReturnStmt*>(function_body->stmt_body.back().get())){
            auto fake_token = token::Token{token::TokenType::IntegerLiteral, "0",{-1,-1,-1,-1},"COMPILER GENERATED TOKEN, SOURCE LINE NOT AVAILABLE"};
            std::unique_ptr<Expr> ret_expr = std::make_unique<Constant>(fake_token);
            function_body->stmt_body.push_back(std::make_unique<ReturnStmt>(std::move(ret_expr)));
        }
    }
    symbol::STable* function_table;
    try{
        function_table = st->new_function_scope_child(f_type.return_type());
    }catch(std::runtime_error& e){
        throw sem_error::FlowError(e.what(),this->tok);
    }
    function_body->analyze(function_table);
    for(const auto& decl : params){
        decl->analyze(function_table->most_recent_child());
    }
    std::optional<token::Token> error_tok;
    if((error_tok = function_table->unmatched_label())!= std::nullopt){
        throw sem_error::STError("Goto with unmatched label",error_tok.value());
    }
}
} //namespace ast
