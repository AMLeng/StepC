#include "parse.h"
#include "type.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <cassert>
#include <string_view>
#include <map>
namespace parse{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{

    //Operator Precedence
    constexpr int ternary_cond_binding_power = 6;
    //Unary operators should bind more tightly than any normal binary op
    constexpr int unary_op_binding_power = 40;
    //Higher prescedence than comma, lower than assignment
    constexpr int func_call_arg_binding_power = 3;
    //left binding is less than right binding for left associativity (+, -)
    //and vice versa for right associativity
    std::map<token::TokenType, std::pair<int, int>> binary_op_binding_power = {{
        {token::TokenType::Star, {25,26}}, {token::TokenType::Div, {25,26}},
        {token::TokenType::Mod, {25,26}}, 
        {token::TokenType::Plus,{23,24}}, {token::TokenType::Minus,{23,24}}, 
        {token::TokenType::LShift,{21,22}}, {token::TokenType::RShift,{21,22}}, 
        {token::TokenType::Less, {19,20}}, {token::TokenType::Greater, {19,20}},
        {token::TokenType::LEq, {19,20}}, {token::TokenType::GEq, {19,20}},
        {token::TokenType::Equal, {17,18}}, {token::TokenType::NEqual, {17,18}},
        {token::TokenType::Amp, {15,16}},
        {token::TokenType::BitwiseXor, {13,14}}, {token::TokenType::BitwiseOr, {11,12}},
        {token::TokenType::And, {9,10}}, {token::TokenType::Or, {7,8}},
        {token::TokenType::Assign, {5,4}},
        {token::TokenType::LSAssign, {5,4}},{token::TokenType::RSAssign, {5,4}},
        {token::TokenType::BOAssign, {5,4}},{token::TokenType::BXAssign, {5,4}},
        {token::TokenType::ModAssign, {5,4}},{token::TokenType::BAAssign, {5,4}},
        {token::TokenType::MultAssign, {5,4}},{token::TokenType::DivAssign, {5,4}},
        {token::TokenType::PlusAssign, {5,4}},{token::TokenType::MinusAssign, {5,4}},
        {token::TokenType::Comma, {1,2}},
    }};

    //Check and throw default unexpected token exception
    void check_token_type(const token::Token& tok, token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + token::string_name(type), tok);
        }
    }
    type::CType convert_specifiers(std::multiset<std::string> specifiers){
        if(specifiers.size() == 0){
            throw std::runtime_error("Failed to parse declaration specifiers");
        }
        type::CType t;
        if(specifiers.size() == 1 && specifiers.find("void") != specifiers.end()){
            return t;
        }
        try{
            return type::from_str_multiset(specifiers);
        }catch(std::runtime_error& e){
            throw;
        }
    }
}//namespace
bool is_specifier(const token::Token& tok){
    return tok.value == "void"
        || tok.value == "char"
        || tok.value == "short"
        || tok.value == "int"
        || tok.value == "long"
        || tok.value == "float"
        || tok.value == "double"
        || tok.value == "signed"
        || tok.value == "unsigned"
        || tok.value == "_Bool";
}

type::CType parse_specifiers(lexer::Lexer& l){
    auto specifier_list = std::multiset<std::string>{};
    auto next_tok = l.peek_token();
    while(next_tok.type == token::TokenType::Keyword){
        if(!is_specifier(next_tok)){
            throw parse_error::ParseError("Expected specifier keyword",next_tok);
        }
        specifier_list.insert(l.get_token().value);
        next_tok = l.peek_token();
    }
    try{
        return convert_specifiers(specifier_list);
    }catch(std::runtime_error& e){
        throw sem_error::TypeError(e.what(), l.peek_token());
    }
}
//Definitions for parsing methods
std::unique_ptr<ast::Constant> parse_constant(lexer::Lexer& l){
    auto constant_value = l.get_token();
    if(!token::matches_type(constant_value, 
                token::TokenType::IntegerLiteral, 
                token::TokenType::FloatLiteral)){
        throw parse_error::ParseError("Expected literal",constant_value);
    }
    return std::make_unique<ast::Constant>(constant_value);
}
    
std::unique_ptr<ast::UnaryOp> parse_unary_op(lexer::Lexer& l){
    auto op_token = l.get_token();
    if(!token::matches_type(op_token,
                token::TokenType::Minus,
                token::TokenType::Plus,
                token::TokenType::BitwiseNot,
                token::TokenType::Amp,
                token::TokenType::Star,
                token::TokenType::Not,
                token::TokenType::Plusplus,//Prefix versions
                token::TokenType::Minusminus)){
        throw parse_error::ParseError("Not valid unary operator",op_token);
    }
    auto expr = parse_expr(l, unary_op_binding_power);
    return std::make_unique<ast::UnaryOp>(op_token,std::move(expr));
}

std::unique_ptr<ast::BinaryOp> parse_binary_op(lexer::Lexer& l, std::unique_ptr<ast::Expr> left, int min_bind_power){
    auto op_token = l.get_token();
    if(binary_op_binding_power.find(op_token.type) == binary_op_binding_power.end()){
        throw parse_error::ParseError("Not valid binary operator",op_token);
    }
    auto right = parse_expr(l, min_bind_power);
    return std::make_unique<ast::BinaryOp>(op_token,std::move(left),std::move(right));
}
std::unique_ptr<ast::Variable> parse_variable(lexer::Lexer& l){
    auto var_tok = l.get_token();
    check_token_type(var_tok, token::TokenType::Identifier);
    return std::make_unique<ast::Variable>(var_tok);
}
std::unique_ptr<ast::Conditional> parse_conditional(lexer::Lexer& l, std::unique_ptr<ast::Expr> cond){
    auto question = l.get_token();
    check_token_type(question, token::TokenType::Question);
    auto true_expr = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto false_expr = parse_expr(l,ternary_cond_binding_power);
    return std::make_unique<ast::Conditional>(question, std::move(cond),std::move(true_expr),std::move(false_expr));
}

std::unique_ptr<ast::FuncCall> parse_function_call(lexer::Lexer& l, std::unique_ptr<ast::Expr> func){
    auto tok = l.get_token();
    check_token_type(tok, token::TokenType::LParen);
    auto args = std::vector<std::unique_ptr<ast::Expr>>{};
    while(l.peek_token().type != token::TokenType::RParen){
        args.push_back(parse_expr(l,func_call_arg_binding_power));
        if(l.peek_token().type == token::TokenType::RParen){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    return std::make_unique<ast::FuncCall>(tok, std::move(func), std::move(args));
}

std::unique_ptr<ast::ArrayAccess> parse_array_access(lexer::Lexer& l, std::unique_ptr<ast::Expr> arg){
    auto op_token = l.get_token();
    check_token_type(op_token, token::TokenType::LBrack);
    auto index = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RBrack);
    return std::make_unique<ast::ArrayAccess>(op_token,std::move(arg), std::move(index));
}
std::unique_ptr<ast::Postfix> parse_postfix(lexer::Lexer& l, std::unique_ptr<ast::Expr> arg){
    auto op_token = l.get_token();
    if(!token::matches_type(op_token,
                token::TokenType::Plusplus,
                token::TokenType::Minusminus)){
        throw parse_error::ParseError("Not valid postfix operator",op_token);
    }
    return std::make_unique<ast::Postfix>(op_token,std::move(arg));
}
std::unique_ptr<ast::Expr> parse_expr(lexer::Lexer& l, int min_bind_power){
    auto expr_start = l.peek_token();
    std::unique_ptr<ast::Expr> expr_ptr = nullptr;
    switch(expr_start.type){
        case token::TokenType::IntegerLiteral:
        case token::TokenType::FloatLiteral:
            expr_ptr =  parse_constant(l);
            break;
        case token::TokenType::Minus:
        case token::TokenType::Plus:
        case token::TokenType::BitwiseNot:
        case token::TokenType::Amp:
        case token::TokenType::Star:
        case token::TokenType::Not:
        case token::TokenType::Plusplus:
        case token::TokenType::Minusminus:
            expr_ptr =  parse_unary_op(l);
            break;
        case token::TokenType::Identifier:
            expr_ptr = parse_variable(l);
            break;
        case token::TokenType::LParen:
            l.get_token();
            expr_ptr = parse_expr(l);
            check_token_type(l.get_token(), token::TokenType::RParen);
            break;
    }
    if(expr_ptr == nullptr){
        throw parse_error::ParseError("Expected beginning of expression",l.peek_token());
    }
    //While the next thing is an operator of high precedence, keep parsing
    while(true){
        auto potential_op_token = l.peek_token();
        if(potential_op_token.type == token::TokenType::Question){//Ternary conditional
            if(ternary_cond_binding_power < min_bind_power){
                break;
            }
            expr_ptr = parse_conditional(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::LParen){
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_function_call(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::LBrack){
            //postfix array access
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_array_access(l, std::move(expr_ptr));
            continue;
        }
        if(potential_op_token.type == token::TokenType::Plusplus ||
            potential_op_token.type == token::TokenType::Minusminus){
            //postfix increment/decrement
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_postfix(l, std::move(expr_ptr));
            continue;
        }
        if(binary_op_binding_power.find(potential_op_token.type) == binary_op_binding_power.end()){
            break; //Not an operator
        }
        auto binding_power = binary_op_binding_power.at(potential_op_token.type);
        if(binding_power.first < min_bind_power){
            break;
        }
        expr_ptr = parse_binary_op(l, std::move(expr_ptr), binding_power.second);
    }
    return expr_ptr;
}

std::unique_ptr<ast::GotoStmt> parse_goto_stmt(lexer::Lexer& l){
    auto goto_keyword = l.get_token();
    check_token_type(goto_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(goto_keyword, "goto")){
        throw parse_error::ParseError("Expected keyword \"goto\"", goto_keyword);
    }
    auto ident_tok = l.get_token();
    check_token_type(ident_tok, token::TokenType::Identifier);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::GotoStmt>(ident_tok);
}
std::unique_ptr<ast::LabeledStmt> parse_labeled_stmt(lexer::Lexer& l){
    auto ident_tok = l.get_token();
    check_token_type(ident_tok, token::TokenType::Identifier);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::LabeledStmt>(ident_tok, std::move(body));
}
std::unique_ptr<ast::BreakStmt> parse_break_stmt(lexer::Lexer& l){
    auto break_keyword = l.get_token();
    check_token_type(break_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(break_keyword, "break")){
        throw parse_error::ParseError("Expected keyword \"break\"", break_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::BreakStmt>(break_keyword);
}
std::unique_ptr<ast::ContinueStmt> parse_continue_stmt(lexer::Lexer& l){
    auto continue_keyword = l.get_token();
    check_token_type(continue_keyword, token::TokenType::Keyword);
    if(!token::matches_keyword(continue_keyword, "continue")){
        throw parse_error::ParseError("Expected keyword \"continue\"", continue_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::ContinueStmt>(continue_keyword);
}
std::unique_ptr<ast::ReturnStmt> parse_return_stmt(lexer::Lexer& l){
    auto return_keyword = l.get_token();
    check_token_type(return_keyword, token::TokenType::Keyword);

    if(!token::matches_keyword(return_keyword, "return")){
        throw parse_error::ParseError("Expected keyword \"return\"", return_keyword);
    }
    if(l.peek_token().type == token::TokenType::Semicolon){
        l.get_token();
        return std::make_unique<ast::ReturnStmt>(return_keyword, std::nullopt);
    }
    auto ret_value = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::ReturnStmt>(return_keyword, std::move(ret_value));
}

std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && is_specifier(next_token)){
        return parse_decl_list(l);
    }
    return parse_stmt(l);
}
std::unique_ptr<ast::Stmt> parse_stmt(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "return")){
        return parse_return_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "if")){
        return parse_if_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "for")){
        return parse_for_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "do")){
        return parse_do_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "while")){
        return parse_while_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "continue")){
        return parse_continue_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "break")){
        return parse_break_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "goto")){
        return parse_goto_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "switch")){
        return parse_switch_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "case")){
        return parse_case_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword && token::matches_keyword(next_token, "default")){
        return parse_default_stmt(l);
    }
    if(next_token.type == token::TokenType::Keyword){
        throw parse_error::ParseError("Unknown keyword in statement beginning", next_token);
    }
    if(next_token.type == token::TokenType::LBrace){
        return parse_compound_stmt(l);
    }
    if(next_token.type == token::TokenType::Semicolon){
        l.get_token();
        return std::make_unique<ast::NullStmt>();
    }
    if(next_token.type == token::TokenType::Identifier){
        auto maybe_colon = l.peek_token(2);
        if(maybe_colon.type == token::TokenType::Colon){
            return parse_labeled_stmt(l);
        }
    }
    //If is a typedef name will also parse var decl, but that's for later
    auto expr = parse_expr(l);
    auto semicolon = l.get_token();
    check_token_type(semicolon, token::TokenType::Semicolon);
    return std::move(expr);
}
std::unique_ptr<ast::CaseStmt> parse_case_stmt(lexer::Lexer& l){
    auto case_keyword = l.get_token();
    if(!token::matches_keyword(case_keyword, "case")){
        throw parse_error::ParseError("Expected keyword \"case\"", case_keyword);
    }
    auto c = parse_constant(l);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::CaseStmt>(case_keyword, std::move(c), std::move(body));
}
std::unique_ptr<ast::DefaultStmt> parse_default_stmt(lexer::Lexer& l){
    auto default_keyword = l.get_token();
    if(!token::matches_keyword(default_keyword, "default")){
        throw parse_error::ParseError("Expected keyword \"default\"", default_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto body = parse_stmt(l);
    return std::make_unique<ast::DefaultStmt>(default_keyword, std::move(body));
}
std::unique_ptr<ast::SwitchStmt> parse_switch_stmt(lexer::Lexer& l){
    auto switch_keyword = l.get_token();
    if(!token::matches_keyword(switch_keyword, "switch")){
        throw parse_error::ParseError("Expected keyword \"switch\"", switch_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto body = parse_stmt(l);
    return std::make_unique<ast::SwitchStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::WhileStmt> parse_while_stmt(lexer::Lexer& l){
    auto while_keyword = l.get_token();
    if(!token::matches_keyword(while_keyword, "while")){
        throw parse_error::ParseError("Expected keyword \"while\"", while_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto body = parse_stmt(l);
    return std::make_unique<ast::WhileStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::DoStmt> parse_do_stmt(lexer::Lexer& l){
    auto do_keyword = l.get_token();
    if(!token::matches_keyword(do_keyword, "do")){
        throw parse_error::ParseError("Expected keyword \"do\"", do_keyword);
    }
    auto body = parse_stmt(l);
    auto while_keyword = l.get_token();
    if(!token::matches_keyword(while_keyword, "while")){
        throw parse_error::ParseError("Expected keyword \"while\"", while_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto control = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    check_token_type(l.get_token(), token::TokenType::Semicolon);
    return std::make_unique<ast::DoStmt>(std::move(control), std::move(body));
}
std::unique_ptr<ast::ForStmt> parse_for_stmt(lexer::Lexer& l){
    auto for_keyword = l.get_token();
    if(!token::matches_keyword(for_keyword, "for")){
        throw parse_error::ParseError("Expected keyword \"for\"", for_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    //Parse initial clause
    auto init = std::variant<std::monostate,std::unique_ptr<ast::DeclList>,std::unique_ptr<ast::Expr>>{};
    if(!token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
        if(token::matches_type(l.peek_token(),token::TokenType::Keyword)){
            init = parse_decl_list(l);
        }else{
            init = parse_expr(l);
        }
    }

    check_token_type(l.get_token(), token::TokenType::Semicolon);
    //Parse control expr
    static const auto fake_token = token::Token{
        token::TokenType::IntegerLiteral, "1",{-1,-1,-1,-1},"COMPILER GENERATED TOKEN, SOURCE LINE NOT AVAILABLE"};
    std::unique_ptr<ast::Expr> control = std::make_unique<ast::Constant>(fake_token);
    if(!token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
        control = parse_expr(l);
    }

    check_token_type(l.get_token(), token::TokenType::Semicolon);
    //Parse post expr
    auto post = std::optional<std::unique_ptr<ast::Expr>>{std::nullopt};
    if(!token::matches_type(l.peek_token(),token::TokenType::RParen)){
        post = parse_expr(l);
    }
    check_token_type(l.get_token(), token::TokenType::RParen);
    //Parse body
    auto body = parse_stmt(l);
    return std::make_unique<ast::ForStmt>(std::move(init), std::move(control), std::move(post), std::move(body));
}
std::unique_ptr<ast::IfStmt> parse_if_stmt(lexer::Lexer& l){
    auto if_keyword = l.get_token();
    if(!token::matches_keyword(if_keyword, "if")){
        throw parse_error::ParseError("Expected keyword \"if\"", if_keyword);
    }
    check_token_type(l.get_token(), token::TokenType::LParen);
    auto if_condition = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::RParen);
    auto if_body = parse_stmt(l);
    auto maybe_else= l.peek_token();
    if(maybe_else.type != token::TokenType::Keyword || !token::matches_keyword(maybe_else, "else")){
        return std::make_unique<ast::IfStmt>(std::move(if_condition), std::move(if_body));
    }
    check_token_type(l.get_token(), token::TokenType::Keyword);
    auto else_body = parse_stmt(l);
    return std::make_unique<ast::IfStmt>(std::move(if_condition), std::move(if_body), std::move(else_body));
}
std::unique_ptr<ast::CompoundStmt> parse_compound_stmt(lexer::Lexer& l){
    auto stmt_body = std::vector<std::unique_ptr<ast::BlockItem>>{};
    check_token_type(l.get_token(), token::TokenType::LBrace);
    while(l.peek_token().type != token::TokenType::RBrace){
        stmt_body.push_back(parse_block_item(l));
    }
    check_token_type(l.get_token(), token::TokenType::RBrace);
    return std::make_unique<ast::CompoundStmt>(std::move(stmt_body));
}
std::unique_ptr<ast::InitializerList> parse_initializer_list(lexer::Lexer& l){
    auto inits = std::vector<std::unique_ptr<ast::Initializer>>{};
    auto tok = l.get_token();
    check_token_type(tok, token::TokenType::LBrace);
    while(l.peek_token().type != token::TokenType::RBrace){
        if(l.peek_token().type == token::TokenType::LBrace){
            inits.push_back(parse_initializer_list(l));
        }else{
            inits.push_back(parse_expr(l,binary_op_binding_power.at(token::TokenType::Assign).second));
        }
        if(token::matches_type(l.peek_token(),token::TokenType::RBrace)){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }
    check_token_type(l.get_token(), token::TokenType::RBrace);
    return std::make_unique<ast::InitializerList>(tok, std::move(inits));
}
std::unique_ptr<ast::Decl> parse_init_decl(lexer::Lexer& l, Declarator declarator){
    auto var_name = declarator.first.value();
    check_token_type(var_name, token::TokenType::Identifier);
    if(l.peek_token().type == token::TokenType::Assign){
        if(type::is_type<type::VoidType>(declarator.second)){
            throw sem_error::TypeError("Invalid type 'void'", var_name);
        }
        if(type::is_type<type::FuncType>(declarator.second)){
            throw sem_error::TypeError("Invalid assignment to function type", var_name);
        }
        l.get_token();
        if(l.peek_token().type == token::TokenType::LBrace){
            auto assign = parse_initializer_list(l);
            return std::make_unique<ast::VarDecl>(var_name, declarator.second, std::move(assign));
        }else{
            auto assign = parse_expr(l,binary_op_binding_power.at(token::TokenType::Assign).second);
            return std::make_unique<ast::VarDecl>(var_name, declarator.second, std::move(assign));
        }
    }else{
        if(type::is_type<type::VoidType>(declarator.second)){
            throw sem_error::TypeError("Invalid type 'void'", var_name);
        }
        if(type::is_type<type::FuncType>(declarator.second)){
            return std::make_unique<ast::FunctionDecl>(var_name, type::get<type::FuncType>(declarator.second));
        }else{
            return std::make_unique<ast::VarDecl>(var_name, declarator.second);
        }
    }
}


std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    auto next = l.peek_token();
    auto global_decls = std::vector<std::unique_ptr<ast::ExtDecl>>{};
    while(next.type != token::TokenType::END){
        global_decls.push_back(parse_ext_decl(l));
        next = l.peek_token();
    }
    return std::make_unique<ast::Program>(std::move(global_decls));
}

} //namespace parse
