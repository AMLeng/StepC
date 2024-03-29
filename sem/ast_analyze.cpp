#include "ast.h"
#include "parse.h"
#include "type.h"
#include "sem_error.h"
#include <array>
#include <sstream>
namespace ast{
namespace{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

const auto assignment_op = std::map<token::TokenType,token::TokenType>{{
    {token::TokenType::Assign, token::TokenType::Assign},
    {token::TokenType::PlusAssign, token::TokenType::Plus},
    {token::TokenType::MinusAssign, token::TokenType::Minus},
    {token::TokenType::MultAssign, token::TokenType::Star},
    {token::TokenType::DivAssign, token::TokenType::Div},
    {token::TokenType::ModAssign, token::TokenType::Mod},
    {token::TokenType::LSAssign, token::TokenType::LShift},
    {token::TokenType::RSAssign, token::TokenType::RShift},
    {token::TokenType::BAAssign, token::TokenType::Amp},
    {token::TokenType::BOAssign, token::TokenType::BitwiseOr},
    {token::TokenType::BXAssign, token::TokenType::BitwiseXor},
}};
bool is_func_designator(const ast::AST* node){
    auto var = dynamic_cast<const ast::Variable*>(node);
    return var && type::is_type<type::PointerType>(var->type)
        && type::is_type<type::FuncType>(type::get<type::PointerType>(var->type).pointed_type());
}
bool is_nullptr_constant(const ast::Expr* node){
    if(type::is_type<type::PointerType>(node->type)){
        auto t = type::get<type::PointerType>(node->type);
        if(!type::is_type<type::VoidType>(t.pointed_type())){
            return false;
        }
    }else{
        if(!type::is_type<type::IType>(node->type)){
            return false;
        }
    }
    return std::visit(overloaded{
        [](std::monostate){return false;},
        [](auto val){return val == 0;},
    }, node->constant_value);
}
std::array<type::CType,3> analyze_bin_op(type::CType left, type::CType right, token::TokenType op, token::Token tok){
    //Returns an array of: {result type, converted left type, converted right type}
    switch(op){
        case token::TokenType::Plus:
            if(type::is_arith(left) && type::is_arith(right)){
                auto type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type, type, type};
            }
            if(type::is_type<type::IType>(left) && type::is_type<type::PointerType>(right)){
                //Splices right in the case that it's an array type
                return std::array<type::CType, 3>{type::get<type::PointerType>(right), left, right};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::IType>(right)){
                //Splices left in the case that it's an array type
                return std::array<type::CType, 3>{type::get<type::PointerType>(left), left, right};
            }
            throw sem_error::TypeError("Invalid types \""+type::to_string(left)+"\" and \""
                +type::to_string(right)+"\" for addition",tok);
        case token::TokenType::Minus:
            if(type::is_arith(left) && type::is_arith(right)){
                auto type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type, type, type};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::PointerType>(right)){
                if(!type::is_compatible(type::get<type::PointerType>(left).pointed_type(),
                        type::get<type::PointerType>(right).pointed_type())){
                    throw sem_error::TypeError("Pointer types being subtracted must be to compatible types",tok);
                }
                //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                return std::array<type::CType, 3>{type::CType(type::IType::LLong), left, right};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::IType>(right)){
                return std::array<type::CType, 3>{type::get<type::PointerType>(left), left, right};
            }
            throw sem_error::TypeError("Invalid types \""+type::to_string(left)+"\" and \""
                +type::to_string(right)+"\" for subtraction",tok);
        case token::TokenType::Star:
        case token::TokenType::Div:
            if(type::is_arith(left) && type::is_arith(right)){
                auto type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type, type, type};
            }
            throw sem_error::TypeError("Operand of arithmetic type required for multiplicative operator",tok);
        case token::TokenType::Mod:
            if(type::is_int(left) && type::is_int(right)){
                auto type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type, type, type};
            }
            throw sem_error::TypeError("Operand of integer type required for modulo",tok);
        case token::TokenType::Amp:
        case token::TokenType::BitwiseOr:
        case token::TokenType::BitwiseXor:
            if(type::is_int(left) && type::is_int(right)){
                auto type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type, type, type};
            }
            throw sem_error::TypeError("Operand of integer type required",tok);
        case token::TokenType::LShift:
        case token::TokenType::RShift:
            if(type::is_int(left) && type::is_int(right)){
                auto promoted_left = type::integer_promotions(left);
                return std::array<type::CType, 3>{promoted_left, promoted_left, type::integer_promotions(right)};
            }
            throw sem_error::TypeError("Operand of integer type required",tok);
        case token::TokenType::And:
        case token::TokenType::Or:
            if(type::is_scalar(left) && type::is_scalar(right)){
                return std::array<type::CType, 3>{type::from_str("int"), left, right};
            }
            throw sem_error::TypeError("Operand of scalar type required",tok);
        case token::TokenType::Equal:
        case token::TokenType::NEqual:
            if(type::is_arith(left) && type::is_arith(right)){
                auto convert_type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type::from_str("int"), convert_type, convert_type};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::IType>(right)){
                return std::array<type::CType, 3>{type::from_str("int"), left, left};
            }
            if(type::is_type<type::IType>(left) && type::is_type<type::PointerType>(right)){
                return std::array<type::CType, 3>{type::from_str("int"), right, right};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::PointerType>(right)){
                auto l = type::get<type::PointerType>(left);
                auto r = type::get<type::PointerType>(right);
                if(l.pointed_type() == type::CType(type::VoidType())){
                    return std::array<type::CType, 3>{type::from_str("int"), l, l};
                }
                if(r.pointed_type() == type::CType(type::VoidType())){
                    return std::array<type::CType, 3>{type::from_str("int"), r, r};
                }
                if(!type::is_compatible(l.pointed_type(),r.pointed_type())){
                    throw sem_error::TypeError("Pointer types being compared for equality must be to compatible types",tok);
                }else{
                    return std::array<type::CType, 3>{type::from_str("int"), l, r};
                }
            }
            throw sem_error::TypeError("Invalid types \""+type::to_string(left)+"\" and \""
                +type::to_string(right)+"\" for equality operator",tok);
        case token::TokenType::Less:
        case token::TokenType::Greater:
        case token::TokenType::LEq:
        case token::TokenType::GEq:
            if(type::is_arith(left) && type::is_arith(right)){
                //Check that they are real types
                auto convert_type = type::usual_arithmetic_conversions(left, right);
                return std::array<type::CType, 3>{type::from_str("int"), convert_type, convert_type};
            }
            if(type::is_type<type::PointerType>(left) && type::is_type<type::PointerType>(right)){
                auto l = type::get<type::PointerType>(left);
                auto r = type::get<type::PointerType>(right);
                if(!type::is_compatible(l.pointed_type(),r.pointed_type())){
                    throw sem_error::TypeError("Pointer types being compared for equality must be to compatible types",tok);
                }else{
                    return std::array<type::CType, 3>{type::from_str("int"), l, r};
                }
            }
            throw sem_error::TypeError("Invalid types \""+type::to_string(left)+"\" and \""
                +type::to_string(right)+"\" for relational operator",tok);
        case token::TokenType::Comma:
            return std::array<type::CType, 3>{left, left, right};
    }
    assert(false && "Unknown binary operator type");
    __builtin_unreachable();
}
ConstantExprType compute_binary_constant(ConstantExprType left, ConstantExprType right, token::TokenType op){
    switch(op){
        case token::TokenType::Plus:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left + right;
                }
            }, left, right);
        case token::TokenType::Minus:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left - right;
                }
            }, left, right);
        case token::TokenType::Star:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left * right;
                }
            }, left, right);
        case token::TokenType::Div:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    if(right == 0){
                        return ConstantExprType();
                    }
                    return left / right;
                }
            }, left, right);
        case token::TokenType::Mod:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    if(right == 0){
                        return ConstantExprType();
                    }
                    return left % right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::Amp:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    return left & right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::BitwiseOr:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    return left | right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::BitwiseXor:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    return left ^ right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::LShift:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    return left << right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::RShift:
            return std::visit(overloaded{
                [](long long int left, long long int right)->ConstantExprType{
                    return left >> right;
                },
                [](auto , auto ){return ConstantExprType();}
            }, left, right);
        case token::TokenType::And:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto left, std::monostate )->ConstantExprType{
                    if(left == 0){
                        return 0;
                    }else{
                        return ConstantExprType();
                    }},
                [](auto left, auto right)->ConstantExprType{
                    if(left == 0){
                        return 0;
                    }else{
                        return right != 0;
                    }
                }
            }, left, right);
        case token::TokenType::Or:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto left, std::monostate )->ConstantExprType{
                    if(left != 0){
                        return 1;
                    }else{
                        return ConstantExprType();
                    }},
                [](auto left, auto right)->ConstantExprType{
                    if(left != 0){
                        return 1;
                    }else{
                        return right != 0;
                    }
                }
            }, left, right);
        case token::TokenType::Equal:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left == right;
                }
            }, left, right);
        case token::TokenType::NEqual:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left != right;
                }
            }, left, right);
        case token::TokenType::Less:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left < right;
                }
            }, left, right);
        case token::TokenType::Greater:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left > right;
                }
            }, left, right);
        case token::TokenType::LEq:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left <= right;
                }
            }, left, right);
        case token::TokenType::GEq:
            return std::visit(overloaded{
                [](std::monostate , std::monostate ){return ConstantExprType();},
                [](std::monostate , auto ){return ConstantExprType();},
                [](auto , std::monostate ){return ConstantExprType();},
                [](auto left, auto right)->ConstantExprType{
                    return left >= right;
                }
            }, left, right);
        default:
            return ConstantExprType();
    }
    __builtin_unreachable();
}
template <typename T>
T lookup_tag(type::CType t, token::Token tok){
    try{
        return type::get<T>(type::CType::get_tag(type::get<T>(t).tag));
    }catch(std::exception& e){
        throw sem_error::STError("Could not find struct with name "+type::get<T>(t).tag,tok);
    }
}
} //namespace
bool is_lval(const ast::AST* node){
    //We assume that arrays will all decay to pointers, so that nothing of array type is an lvalue
    if(const auto p = dynamic_cast<const ast::Variable*>(node)){
        return !type::is_type<type::ArrayType>(p->type);
    }
    if(dynamic_cast<const ast::StrLiteral*>(node)){
        return true;
    }
    if(const auto p = dynamic_cast<const ast::UnaryOp*>(node)){
        if(p->tok.type == token::TokenType::Star){
            return true;
        }
    }
    if(dynamic_cast<const ast::ArrayAccess*>(node)){
        return true;
    }
    if(const auto p = dynamic_cast<const ast::MemberAccess*>(node)){
        return is_lval(p->arg.get());
    }
    return false;
}

void AmbiguousBlock::analyze(symbol::STable* st){
    auto input = std::stringstream{};
    lexer::TokenStream l(this->unparsed_tokens);
    if(!st->has_symbol(this->ambiguous_ident.value)){
        throw sem_error::STError("Could not find identifier "+ambiguous_ident.value+" in symbol table", this->ambiguous_ident);
    }
    if(st->resolves_to_typedef(this->ambiguous_ident.value)){
        parsed_item = parse::parse_decl_list(l);
    }else{
        parsed_item = parse::parse_stmt(l);
    }
    assert(parsed_item && "Failed to resolve ambiguity in block item");
    parsed_item->analyze(st);
}

void TypedefDecl::analyze(symbol::STable* st) {
    try{
        st->add_typedef(this->name, this->type);
    }catch(std::exception& e){
        throw sem_error::STError("Error adding typedef definition for "+name+" to symbol table:\n"+e.what(), this->tok);
    }
}
void TagDecl::analyze(symbol::STable* st) {
    std::string name = std::visit(type::overloaded{
        [](const auto& t){
            return t.tag;
        },
    }, this->type);
    try{
        st->add_tag(name, this->type);
    }catch(std::exception& e){
        throw sem_error::STError("Error adding tag definition "+name+" to symbol table:\n"+e.what(), this->tok);
    }
}
void EnumVarDecl::analyze(symbol::STable* st) {
    this->initializer->analyze(st);
    if(!type::can_assign(this->initializer->type,type::IType::Int)){
        throw sem_error::TypeError("Invalid type "+type::to_string(this->initializer->type)+" for initializing enum member",tok);
    }
    if(!std::holds_alternative<long long int>(this->initializer->constant_value)){
        throw sem_error::FlowError("Enum member definition must be constant",this->tok);
    }
    //Add symbol to symbol table, check that not already present
    try{
        st->add_constant(this->tok.value,std::get<long long int>(this->initializer->constant_value));
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
}
void VarDecl::analyze(symbol::STable* st) {
    this->analyzed = true;
    try{
        this->type = st->mangle_type_or_throw(this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    if(type::is_type<type::ArrayType>(this->type)){
        auto array_type = type::get<type::ArrayType>(this->type);
        if(!this->assignment.has_value() && !array_type.is_complete()){
            throw sem_error::TypeError("Cannot infer size of declared array without initialization",this->tok);
        }
    }
    //If we have a declaration attached
    if(this->assignment.has_value()){
        this->assignment.value()->initializer_analyze(this->type, st);
    }
    //Add symbol to symbol table, check that not already present
    try{
        st->add_symbol(this->name,this->type, this->assignment.has_value());
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
}
void Expr::initializer_analyze(type::CType& variable_type, symbol::STable* st){
    this->analyze(st);
    if(!type::can_assign(this->type,variable_type) 
            && !(type::is_type<type::PointerType>(variable_type) && is_nullptr_constant(this))){
        throw sem_error::TypeError("Invalid types "+type::to_string(this->type)+" and "+type::to_string(variable_type)+" for initialization",tok);
    }
    if(!st->in_function()){
        //If not in function, is global and needs to be constant
        if(std::holds_alternative<std::monostate>(this->constant_value) && !dynamic_cast<ast::StrLiteral*>(this)){
            throw sem_error::FlowError("Global variable def must be constant",this->tok);
        }
    }
    if(type::is_type<type::ArrayType>(variable_type) && type::is_type<type::ArrayType>(this->type)){
        auto array_type = type::get<type::ArrayType>(variable_type);
        auto expr_type = type::get<type::ArrayType>(this->type);
        if(!array_type.is_complete() && expr_type.is_complete()){
            array_type.set_size(expr_type.size());
            variable_type = array_type;
        }
    }
}
void InitializerList::initializer_analyze(type::CType& variable_type, symbol::STable* st){
    auto length = initializers.size();
    if(type::is_type<type::ArrayType>(variable_type)){
        auto array_type = type::get<type::ArrayType>(variable_type);
        auto element_type = array_type.pointed_type();
        if(array_type.is_complete() && array_type.size() < length){
            length = array_type.size();
        }
        if(!array_type.is_complete()){
            array_type.set_size(length);
            variable_type = array_type;
        }
        for(int i=0; i<length; i++){
            initializers.at(i)->initializer_analyze(element_type, st);
        }
    }else if(type::is_type<type::StructType>(variable_type)){
        auto s_type = lookup_tag<type::StructType>(variable_type, tok);
        if(s_type.members.size() < length){
            length = s_type.members.size();
        }
        for(int i=0; i<length; i++){
            initializers.at(i)->initializer_analyze(s_type.members.at(i), st);
        }
    }else if(type::is_type<type::UnionType>(variable_type)){
        auto u_type = lookup_tag<type::UnionType>(variable_type, tok);
        if(u_type.members.size() > 0){
            initializers.front()->initializer_analyze(u_type.members.front(), st);
        }
    }else{
        if(length == 0){
            throw sem_error::TypeError("Cannot have empty initializer for scalar", this->tok);
        }
        initializers.front()->initializer_analyze(variable_type, st);
    }
}
void Variable::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Check that the variable name actually exists in a symbol table
    if(!st->has_symbol(this->variable_name)){
        throw sem_error::STError("Variable not found in symbol table",this->tok);
    }
    auto type_in_table = st->symbol_type(this->variable_name);
    if(type::is_type<type::VoidType>(type_in_table)){
        throw sem_error::STError("Variable cannot have void type",this->tok);
    }
    if(type::is_type<type::FuncType>(type_in_table)){
        this->type = type::PointerType(type_in_table);
        return;
    }
    this->type = type_in_table;
    if(st->resolves_to_constant(this->variable_name)){
        this->constant_value = st->get_constant_value(this->variable_name);
    }
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

    std::visit(overloaded{
        [&](std::monostate ){},
        [&](auto val){
            if(val == 0){
                if(!std::holds_alternative<std::monostate>(false_expr->constant_value)){
                    this->constant_value = false_expr->constant_value;
                }
            }else{
                if(!std::holds_alternative<std::monostate>(true_expr->constant_value)){
                    this->constant_value = true_expr->constant_value;
                }
            }
        },
    }, this->cond->constant_value);
}
void Sizeof::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    this->type = type::IType::LLong;
    this->constant_value = type::size(arg->type);
}
void Alignof::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    this->type = type::IType::LLong;
    this->constant_value = type::align(arg->type);
}
void FuncCall::analyze(symbol::STable* st) {
    this->analyzed = true;
    //Check that the function actually exists in a symbol table
    this->func->analyze(st);
    auto arg_types = std::vector<type::CType>{};
    for(auto& expr : args){
        expr->analyze(st);
        arg_types.push_back(expr->type);
    }
    if(arg_types.size() == 0){
        arg_types.push_back(type::CType());
    }
    auto original_type = this->func->type;
    if(type::is_type<type::PointerType>(original_type)){
        original_type = type::get<type::PointerType>(original_type).pointed_type();
    }
    try{
        auto f_type = type::get<type::FuncType>(original_type);
        if(!f_type.params_match(arg_types)){
            auto error_str = "Cannot call function of type \""+type::to_string(f_type)+"\" on types of provided arguments:\n";
            for(const auto& arg : arg_types){
                error_str += type::to_string(arg)+"\n";
            }
            throw sem_error::TypeError(error_str,this->tok);
        }
        this->type = f_type.return_type();
    }catch(std::runtime_error& e){ //Won't catch the STError
        throw sem_error::STError("Function call with expression not referring to a function or function pointer",this->tok);
    }
}
void MemberAccess::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    if(type::is_type<type::StructType>(this->arg->type)){
        auto s_type = lookup_tag<type::StructType>(this->arg->type, tok);
        try{
            this->type = s_type.members.at(s_type.indices.at(this->index));
        }catch(std::exception& e){
            throw sem_error::TypeError("Could not access member "+index+" in struct with name "+s_type.tag,tok);
        }
        return;
    }
    if(type::is_type<type::UnionType>(this->arg->type)){
        auto u_type = lookup_tag<type::UnionType>(this->arg->type, tok);
        try{
            this->type = u_type.members.at(u_type.indices.at(this->index));
        }catch(std::exception& e){
            throw sem_error::TypeError("Could not access member "+index+" in struct with name "+u_type.tag,tok);
        }
        return;
    }
    throw sem_error::TypeError("Can only perform member access on struct or union type",tok);
}
void ArrayAccess::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    if(!type::is_type<type::PointerType>(this->arg->type)){
        throw sem_error::TypeError("Can only perform array access on pointer type",tok);
    }
    if(type::is_type<type::PointerType>(this->arg->type)){
        this->type = type::get<type::PointerType>(this->arg->type).pointed_type();
    }
    this->index->analyze(st);
    if(!type::is_type<type::IType>(this->index->type)){
        throw sem_error::TypeError("Array index required to be an integer",tok);
    }
}
void Postfix::analyze(symbol::STable* st) {
    this->analyzed = true;
    this->arg->analyze(st);
    //Typechecking
    switch(this->tok.type){
        case token::TokenType::Plusplus:
            if(!is_lval(this->arg.get())){
                throw sem_error::TypeError("Lvalue required as argument of increment",tok);
            }
            if(type::is_arith(this->arg->type)){
                this->type = type::integer_promotions(this->arg->type);
                break;
            }
            if(type::is_type<type::PointerType>(this->arg->type)){
                this->type = type::get<type::PointerType>(this->arg->type);
                break;
            }
            throw sem_error::TypeError("Operand of real or pointer type required",tok);
        case token::TokenType::Minusminus:
            if(!is_lval(this->arg.get())){
                throw sem_error::TypeError("Lvalue required as argument of decrement",tok);
            }
            if(type::is_arith(this->arg->type)){
                this->type = type::integer_promotions(this->arg->type);
                break;
            }
            if(type::is_type<type::PointerType>(this->arg->type)){
                this->type = type::get<type::PointerType>(this->arg->type);
                break;
            }
            throw sem_error::TypeError("Operand of real or pointer type required",tok);
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
        case token::TokenType::Star:
            if(!type::is_type<type::PointerType>(this->arg->type)){
                throw sem_error::TypeError("Cannot dereference non-pointer type",tok);
            }
            this->type = type::get<type::PointerType>(this->arg->type).pointed_type();
            if(type::is_type<type::VoidType>(this->type)){
                throw sem_error::TypeError("Cannot dereference void pointer",tok);
            }
            if(type::is_type<type::FuncType>(this->type)){
                //Dereferencing a function pointer just gives a function pointer
                this->type = this->arg->type;
            }
            break;
        case token::TokenType::Amp:
            {
                //Must come before lvalue check, since this is an exception where arrays are lvalues
                auto p= dynamic_cast<ast::Variable*>(this->arg.get());
                if(p && type::is_type<type::ArrayType>(p->type)){
                    this->type = type::PointerType(p->type);
                    return;
                }
            }
            if(!is_lval(this->arg.get())){
                throw sem_error::TypeError("Lvalue required as argument of address operator",tok);
            }
            if(is_func_designator(this->arg.get())){
                //Taking the address of a function designator does nothing
                this->type = this->arg->type;
                break;
            }
            //If is lvalue, should check that not bitfield and not of register type
            this->type = type::PointerType(this->arg->type);
            break;
        case token::TokenType::Plusplus:
            if(!is_lval(this->arg.get())){
                throw sem_error::TypeError("Lvalue required as argument of increment",tok);
            }
            if(type::is_arith(this->arg->type)){
                this->type = type::integer_promotions(this->arg->type);
                break;
            }
            if(type::is_type<type::PointerType>(this->arg->type)){
                this->type = type::get<type::PointerType>(this->arg->type);
                break;
            }
            throw sem_error::TypeError("Operand of real or pointer type required",tok);
        case token::TokenType::Minusminus:
            if(!is_lval(this->arg.get())){
                throw sem_error::TypeError("Lvalue required as argument of decrement",tok);
            }
            if(type::is_arith(this->arg->type)){
                this->type = type::integer_promotions(this->arg->type);
                break;
            }
            if(type::is_type<type::PointerType>(this->arg->type)){
                this->type = type::get<type::PointerType>(this->arg->type);
                break;
            }
            throw sem_error::TypeError("Operand of real or pointer type required",tok);
        case token::TokenType::Plus:
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",this->arg->tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            this->constant_value=this->arg->constant_value;
            break;
        case token::TokenType::Minus:
            if(!type::is_arith(this->arg->type)){
                throw sem_error::TypeError("Operand of arithmetic type required",this->arg->tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            std::visit(type::overloaded{
                [&](std::monostate ){},
                [&](auto val){this->constant_value = -val;},
            }, this->arg->constant_value);
            break;
        case token::TokenType::Not:
            if(!type::is_scalar(this->arg->type)){
                throw sem_error::TypeError("Operand of scalar type required",this->arg->tok);
            }
            this->type = type::from_str("int");
            std::visit(type::overloaded{
                [&](std::monostate ){},
                [&](auto val){this->constant_value = !val;},
            }, this->arg->constant_value);
            break;
        case token::TokenType::BitwiseNot:
            if(!type::is_int(this->arg->type)){
                throw sem_error::TypeError("Operand of integer type required", this->arg->tok);
            }
            this->type = type::integer_promotions(this->arg->type);
            std::visit(type::overloaded{
                [&](std::monostate ){},
                [&](long double ){assert(false && "Bitwise not must be applied to an integer");},
                [&](long long int val){this->constant_value = ~val;},
            }, this->arg->constant_value);
            break;
        default:
            assert(false && "Unknown unary operator type");
    }
}
void BinaryOp::analyze(symbol::STable* st){
    this->analyzed = true;
    this->left->analyze(st);
    this->right->analyze(st);
    if(assignment_op.find(this->tok.type) == assignment_op.end()){
        //Non-assignment case
        auto types = analyze_bin_op(this->left->type,this->right->type,this->tok.type, this->tok);
        this->constant_value = compute_binary_constant(this->left->constant_value, this->right->constant_value, this->tok.type);
        this->type = types[0];
        this->new_left_type=types[1];
        this->new_right_type=types[2];
        if(token::matches_type(this->tok, token::TokenType::Equal, token::TokenType::NEqual)){
            //For null ptr constants, we can't do the checking just from the argument types
            if(type::is_type<type::PointerType>(this->left->type) && type::is_type<type::IType>(this->right->type)){
                if(is_nullptr_constant(this->right.get())){
                    this->new_left_type = type::get<type::PointerType>(this->left->type);
                    this->new_right_type = this->left->type;
                }else{
                    throw sem_error::TypeError("Cannot compare pointer with int other than null ptr constant", this->tok);
                }
            }
            if(type::is_type<type::PointerType>(this->right->type) && type::is_type<type::IType>(this->left->type)){
                if(is_nullptr_constant(this->left.get())){
                    this->new_left_type = this->right->type;
                    this->new_right_type = type::get<type::PointerType>(this->right->type);
                }else{
                    throw sem_error::TypeError("Cannot compare pointer with int other than null ptr constant", this->tok);
                }
            }
        }
    }else{
        //Assignment case
        this->type = this->left->type; //Since we assign, this type will be predetermined

        this->new_left_type = this->left->type;
        this->new_right_type = this->right->type;
        if(this->tok.type != token::TokenType::Assign){
            auto types = analyze_bin_op(this->left->type,this->right->type,assignment_op.at(this->tok.type), this->tok);
            this->new_left_type=types[1];
            this->new_right_type=types[2];
        }
        if(!is_lval(this->left.get())){
            throw sem_error::TypeError("Lvalue required on left hand side of assignment",tok);
        }
        if(!type::can_assign(this->right->type,this->left->type) 
            && !(type::is_type<type::PointerType>(this->left->type) && is_nullptr_constant(this->right.get()))){
            throw sem_error::TypeError("Invalid types "+type::to_string(this->right->type)+
                " and "+type::to_string(this->left->type)+" for assignment",tok);
        }
    }
}
void NullStmt::analyze(symbol::STable* st){
}
void StrLiteral::analyze(symbol::STable* st){
    this->analyzed = true;
    this->type = type::ArrayType(type::IType::Char, this->literal.size());
}
void Constant::analyze(symbol::STable* st){
    this->analyzed = true;
    if(type::is_type<type::BasicType>(this->type)){
        std::visit(type::overloaded{
            [&](type::IType){this->constant_value =  std::stoll(this->literal);},
            [&](type::FType){this->constant_value = std::stold(this->literal);}
        }, type::get<type::BasicType>(this->type));
    }

}
void IfStmt::analyze(symbol::STable* st){
    this->if_condition->analyze(st);
    this->if_body->analyze(st);
    if(this->else_body.has_value()){
        this->else_body.value()->analyze(st);
    }
}
void ReturnStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Return statement outside of block");
    auto ret_type = type::CType(type::VoidType());
    if(return_expr.has_value()){
        return_expr.value()->analyze(st);
        if(type::is_type<type::VoidType>(return_expr.value()->type)){
            throw sem_error::TypeError("Cannot have expression with void type in return statement",this->return_expr.value()->tok);
        }
        ret_type = this->return_expr.value()->type;
    }
    if(!type::can_assign(ret_type,bt->return_type())){
        if(this->return_expr.has_value()){
            throw sem_error::TypeError("Invalid return type "
                +type::to_string(ret_type) +" (expected "+type::to_string(bt->return_type()) +")",this->return_expr.value()->tok);
        }else{
            throw sem_error::TypeError("Invalid return type "
                +type::to_string(ret_type) +" (expected "+type::to_string(bt->return_type()) +")",this->tok);
        }
    }
}
void ContinueStmt::analyze(symbol::STable* st){
    if(!st->in_loop){
        throw sem_error::FlowError("Continue statement outside of loop",this->tok);
    }
}
void GotoStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Goto statement outside of block");
    bt->require_label(ident_tok);
    //Matched with a call at function end to check that the label got supplied
}
void LabeledStmt::analyze(symbol::STable* st){
    stmt->analyze(st);
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Labeled statement outside of block");
    try{
        bt->add_label(ident_tok.value);
    }catch(std::runtime_error& e){
        throw sem_error::STError("Duplicate label name within the same function",this->ident_tok);
    }
}
void BreakStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Break statement outside of block");
    if(!st->in_loop && !bt->in_switch()){
        throw sem_error::FlowError("Break statement outside of loop or switch",this->tok);
    }
}
void Program::analyze(symbol::STable* st) {
    for(auto& decl : decls){
        decl->analyze(st);
    }
}
void ForStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "For statement outside of block");
    auto stmt_table = bt->new_block_scope_child();

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
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Case statement outside of block");
    if(!bt->in_switch()){
        throw sem_error::FlowError("Case statement outside of switch",this->tok);
    }
    this->label->analyze(st);
    if(!type::is_int(label->type)){
        throw sem_error::TypeError("Case label must have integer type",this->tok);
    }
    if(!std::holds_alternative<long long int>(label->constant_value)){
        throw sem_error::TypeError("Case label must have constant integer type",this->tok);
    }
    unsigned long long int case_val = std::get<long long int>(label->constant_value);
    try{
        bt->add_case(case_val);
    }catch(std::runtime_error& e){
        throw sem_error::STError("Duplicate case statement in switch",this->label->tok);
    }
    this->stmt->analyze(st);
}
void DefaultStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Case statement outside of block");
    if(!bt->in_switch()){
        throw sem_error::FlowError("Case statement outside of switch",this->tok);
    }
    try{
        bt->add_case(std::nullopt);
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
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Case statement outside of block");
    this->control_type = type::integer_promotions(this->control_expr->type);
    auto stmt_table = bt->new_switch_scope_child();
    switch_body->analyze(stmt_table);
    case_table = stmt_table->transfer_switch_table();
}
void WhileStmt::analyze(symbol::STable* st){
    control_expr->analyze(st);
    if(!type::is_scalar(this->control_expr->type)){
        throw sem_error::TypeError("Condition of scalar type required in for statement control expression",this->control_expr->tok);
    }
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "While statement outside of block");
    auto stmt_table = bt->new_block_scope_child();
    stmt_table->in_loop = true;
    body->analyze(stmt_table);
}
void DoStmt::analyze(symbol::STable* st){
    control_expr->analyze(st);
    if(!type::is_scalar(this->control_expr->type)){
        throw sem_error::TypeError("Condition of scalar type required in for statement control expression",this->control_expr->tok);
    }
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Case statement outside of block");
    auto stmt_table = bt->new_block_scope_child();
    stmt_table->in_loop = true;
    body->analyze(stmt_table);
}
void CompoundStmt::analyze(symbol::STable* st){
    auto bt = dynamic_cast<symbol::BlockTable*>(st);
    assert(bt && "Case statement outside of block");
    auto stmt_table = bt->new_block_scope_child();
    for(auto& stmt : stmt_body){
        stmt->analyze(stmt_table);
    }
}
void DeclList::analyze(symbol::STable* st){
    this->analyzed = true;
    for(const auto& t : tag_decls){
        t->analyze(st);
    }
    for(auto& decl : decls){
        decl->analyze(st);
    }
}
void FunctionDecl::analyze(symbol::STable* st){
    this->analyzed = true;
    try{
        this->type = st->mangle_type_or_throw(this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    if(st->in_function()){
        /*if(this->type is marked with storage specifier other than extern){
            throw sem_error::TypeError("Non extern function declaration inside function",this->tok);
        }*/
        st->add_extern_decl(this->name, this->type);
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
    for(const auto& t : tag_decls){
        t->analyze(st);
    }
    try{
        this->type = st->mangle_type_or_throw(this->type);
    }catch(std::runtime_error& e){
        throw sem_error::STError(e.what(),this->tok);
    }
    auto f_type = type::get<type::FuncType>(this->type);
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
    }
    auto global = dynamic_cast<symbol::GlobalTable*>(st);
    auto function_table = global->new_function_scope_child(f_type.return_type());
    for(const auto& decl : params){
        decl->analyze(function_table);
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
