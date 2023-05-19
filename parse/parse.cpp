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
    //left binding is less than right binding for left associativity (+, -)
    //and vice versa for right associativity
    std::map<token::TokenType, std::pair<int, int>> binary_op_binding_power = {{
        {token::TokenType::Mult, {25,26}}, {token::TokenType::Div, {25,26}},
        {token::TokenType::Mod, {25,26}}, 
        {token::TokenType::Plus,{23,24}}, {token::TokenType::Minus,{23,24}}, 
        {token::TokenType::LShift,{21,22}}, {token::TokenType::RShift,{21,22}}, 
        {token::TokenType::Less, {19,20}}, {token::TokenType::Greater, {19,20}},
        {token::TokenType::LEq, {19,20}}, {token::TokenType::GEq, {19,20}},
        {token::TokenType::Equal, {17,18}}, {token::TokenType::NEqual, {17,18}},
        {token::TokenType::BitwiseAnd, {15,16}},
        {token::TokenType::BitwiseXor, {13,14}}, {token::TokenType::BitwiseOr, {11,12}},
        {token::TokenType::And, {9,10}}, {token::TokenType::Or, {7,8}},
        {token::TokenType::Assign, {5,4}},
        {token::TokenType::LSAssign, {5,4}},{token::TokenType::RSAssign, {5,4}},
        {token::TokenType::BOAssign, {5,4}},{token::TokenType::BXAssign, {5,4}},
        {token::TokenType::ModAssign, {5,4}},{token::TokenType::BAAssign, {5,4}},
        {token::TokenType::MultAssign, {5,4}},{token::TokenType::DivAssign, {5,4}},
        {token::TokenType::PlusAssign, {5,4}},{token::TokenType::MinusAssign, {5,4}},
        {token::TokenType::Comma, {2,3}},
    }};

    //Check and throw default unexpected token exception
    void check_token_type(const token::Token& tok, token::TokenType type){
        if(tok.type != type){
            throw parse_error::ParseError("Expected " + token::string_name(type), tok);
        }
    }
}//namespace

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
std::unique_ptr<ast::LValue> parse_lvalue(lexer::Lexer& l){
    //Right now variables are the only implemented lvalue
    return parse_variable(l);
}
std::unique_ptr<ast::Conditional> parse_conditional(lexer::Lexer& l, std::unique_ptr<ast::Expr> cond){
    auto question = l.get_token();
    check_token_type(question, token::TokenType::Question);
    auto true_expr = parse_expr(l);
    check_token_type(l.get_token(), token::TokenType::Colon);
    auto false_expr = parse_expr(l,ternary_cond_binding_power);
    return std::make_unique<ast::Conditional>(question, std::move(cond),std::move(true_expr),std::move(false_expr));
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
        case token::TokenType::Not:
        case token::TokenType::Plusplus:
        case token::TokenType::Minusminus:
            expr_ptr =  parse_unary_op(l);
            break;
        case token::TokenType::Identifier:
            expr_ptr = parse_lvalue(l);
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
            break;
        }
        if(potential_op_token.type == token::TokenType::Plusplus ||
            potential_op_token.type == token::TokenType::Minusminus){
            //postfix increment/decrement
            if(unary_op_binding_power+1 < min_bind_power){
                break;
            }
            expr_ptr = parse_postfix(l, std::move(expr_ptr));
            break;
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
    auto ret_value = parse_expr(l);
    auto semicolon = l.get_token();
    check_token_type(semicolon, token::TokenType::Semicolon);
    return std::make_unique<ast::ReturnStmt>(std::move(ret_value));
}
std::unique_ptr<ast::DeclList> parse_decl_list(lexer::Lexer& l){
    auto keyword_list = std::multiset<std::string>{};
    auto first_keyword = l.peek_token();
    while(l.peek_token().type == token::TokenType::Keyword){
        keyword_list.insert(l.get_token().value);
    }
    if(keyword_list.size() == 0){
        throw parse_error::ParseError("Parsing decl that did not start with a keyword", first_keyword);
    }
    type::BasicType t;
    check_token_type(l.peek_token(), token::TokenType::Identifier);
    try{
        t = type::from_str_multiset(keyword_list);
    }catch(std::runtime_error& e){
        throw sem_error::TypeError(e.what(), first_keyword);
    }
    auto decls = std::vector<std::unique_ptr<ast::Decl>>{};
    do{
        auto var_name = l.get_token();
        check_token_type(var_name, token::TokenType::Identifier);
        auto next_tok = l.peek_token();
        if(token::matches_type(next_tok, token::TokenType::Assign)){
            std::unique_ptr<ast::LValue> var = std::make_unique<ast::Variable>(var_name);
            auto assign = parse_binary_op(l,std::move(var),
                    binary_op_binding_power.at(token::TokenType::Assign).second);
            decls.push_back(std::make_unique<ast::VarDecl>(var_name, t, std::move(assign)));
        }else{
            decls.push_back(std::make_unique<ast::VarDecl>(var_name, t));
        }
        if(token::matches_type(l.peek_token(),token::TokenType::Semicolon)){
            break;
        }
        check_token_type(l.get_token(), token::TokenType::Comma);
    }while(true);
    return std::make_unique<ast::DeclList>(std::move(decls));
}

std::unique_ptr<ast::BlockItem> parse_block_item(lexer::Lexer& l){
    auto next_token = l.peek_token();
    if(next_token.type == token::TokenType::Keyword && !token::matches_keyword(next_token, 
        "return", "if", "for", "do", "while", "continue", "break", "goto")){
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

std::unique_ptr<ast::FunctionDef> parse_function_def(lexer::Lexer& l){
    auto ret_type = l.get_token();
    if(ret_type.type != token::TokenType::Keyword && ret_type.type != token::TokenType::Identifier){
        throw parse_error::ParseError("Invalid function return type", ret_type);
    }
    auto name = l.get_token();
    if(name.type != token::TokenType::Identifier){
        throw parse_error::ParseError("Invalid function name", name);
    }

    check_token_type(l.get_token(), token::TokenType::LParen);
    check_token_type(l.get_token(), token::TokenType::RParen);
    check_token_type(l.peek_token(), token::TokenType::LBrace);

    auto function_body = parse_compound_stmt(l);

    return std::make_unique<ast::FunctionDef>(name.value, type::from_str(ret_type.value), std::move(function_body));
}

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l){
    auto main_method = parse_function_def(l);
    auto next = l.get_token();
    if(next.type != token::TokenType::END){
        throw parse_error::ParseError("Expected end of file", next);
    }
    return std::make_unique<ast::Program>(std::move(main_method));
}

} //namespace parse
