#include "token_stream.h"
#include "preprocessor.h"
#include "lexer_error.h"
#include <iostream>
#include <cassert>
namespace lexer{
namespace{
template <typename EnhancedTokContainer>
token::Token pop_front(EnhancedTokContainer& tokens){
    auto f = tokens.front();
    tokens.pop_front();
    return f.base;
}
template <typename Iter>
Iter next_nonspace_in_line(const Iter& start){
    auto iter = std::next(start);
    for(;iter->type == token::TokenType::SPACE || iter->type == token::TokenType::COMMENT;iter++){
        if(iter->type == token::TokenType::NEWLINE){
            throw lexer_error::PreprocessorError("Encountered end of line when parsing preprocessor directive ", *iter);
        }
    }
    return iter;
}

} //anon namespace

bool is_directive(const std::string& s){
    return s == "define"
        || s == "undef"
        || s == "ifdef"
        || s == "ifndef"
        || s == "if"
        || s == "endif"
        || s == "else"
        || s == "elif"
        || s == "line"
        || s == "error"
        || s == "include"
        || s == "pragma";
}

EnhancedToken::EnhancedToken(std::pair<token::TokenType,std::string> replacement_data, const EnhancedToken& token_to_expand) 
    : base(token_to_expand.base), disabled(token_to_expand.disabled), can_ignore(replacement_data.first != token::TokenType::Identifier) {
    assert(!token_to_expand.can_ignore && "Should not be macro expanding a token that is set to be ignored");
    this->base.type = replacement_data.first;
    this->base.value = replacement_data.second;
    this->disabled.insert(token_to_expand.base.value);
}

struct Preprocessor::MacroTable{
    std::unordered_map<std::string,std::vector<std::pair<token::TokenType,std::string>>> macro_replacements;
    std::unordered_map<std::string,std::vector<std::string>> function_args;
    bool is_object(const std::string& s);
    bool is_function(const std::string& s);
    bool is_macro(const std::string& s);
};

Preprocessor::Preprocessor(TokenStream& s) : stream(s) {
    table = std::make_unique<MacroTable>();
}
Preprocessor::~Preprocessor() = default;

bool Preprocessor::MacroTable::is_object(const std::string& s){
    return is_macro(s) && !is_function(s);
}
bool Preprocessor::MacroTable::is_function(const std::string& s){
    return this->function_args.find(s) != this->function_args.end();
}
bool Preprocessor::MacroTable::is_macro(const std::string& s){
    return this->macro_replacements.find(s) != this->macro_replacements.end();
}


void Preprocessor::expand_macros(decltype(tokens.begin()) start, decltype(tokens.end()) end){
    if(start == end){
        return;
    }
    if(start->can_ignore){
        return expand_macros(std::next(start), end);
    }
    if(!table->is_macro(start->base.value) || start->disabled.count(start->base.value) > 0 ){
        start->can_ignore = true;
        return expand_macros(std::next(start), end);
    }
    if(table->is_function(start->base.value)){
        assert(false && "Function-like macros not yet implemented");
    }else{
        assert(table->is_object(start->base.value));
        auto new_tokens = std::list<EnhancedToken>{};
        for(const auto& type_value_pair : table->macro_replacements.at(start->base.value)){
            new_tokens.emplace_back(type_value_pair, *start);
        }
        start = this->tokens.erase(start);
        //Inserts tokens in front of ``start''
        this->tokens.insert(start, new_tokens.begin(), new_tokens.end());
        return expand_macros(start, end);
    }
}
void Preprocessor::process_directive(){
    //Assumes that stream.peek_token() is a Hash token
    auto toks = std::list<token::Token>{};
    do{
        toks.push_back(stream.get_token());
    }while(toks.back().type != token::TokenType::NEWLINE);
    //We keep the newline at the end to give us an natural choice of token for if the parsing is unexpectedly
    //Interrupted by the line ending
    assert(toks.front().type == token::TokenType::Hash);
    assert(toks.size() > 2);
    auto directive = next_nonspace_in_line(toks.begin());
    if(!is_directive(directive->value)){
        throw lexer_error::PreprocessorError("Unknown preprocessor directive", *directive);
    }

    //At this point, toks contains the entire line of tokens for the directive
    //And directive contains the token with the name of the actual directive
    //So we can start actually parsing the directive
    if(directive->value == "define"){
        auto current = next_nonspace_in_line(directive);
        const auto ident_token = *current;
        if(ident_token.type == token::TokenType::NEWLINE){
            throw lexer_error::PreprocessorError("Missing identifier for \"define\" preprocessor directive",ident_token);
        }
        current++;
        if(current->type == token::TokenType::LParen){
            //Parse function-like macro arguments
            current++;
            auto args = std::vector<std::string>{};
            while(current->type == token::TokenType::Identifier){
                args.push_back(current->value);
                current++;
                if(current->type == token::TokenType::Comma){
                    current++;
                }else{
                    break;
                }
            }
            if(current->type != token::TokenType::RParen){
                throw lexer_error::PreprocessorError("Unexpected token type in function macro parameter list",*current);
            }
            auto insert_successful = table->function_args.emplace(ident_token.value,std::move(args)).second;
            if(!insert_successful){
                throw lexer_error::PreprocessorError("Function identifier "+ident_token.value +" already defined in preprocessor",ident_token);
            }
            current++;
        }
        auto replacements = std::vector<std::pair<token::TokenType,std::string>>{};
        while(current->type != token::TokenType::NEWLINE){
            replacements.emplace_back(current->type, current->value);
            current++;
        }
        bool insert_successful = table->macro_replacements.emplace(ident_token.value,replacements).second;
        if(!insert_successful){
            throw lexer_error::PreprocessorError("Identifier "+ident_token.value +" already defined in preprocessor",ident_token);
        }
    }else{
        throw lexer_error::PreprocessorError("Unknown preprocessor directive", *directive);
    }
}

token::Token Preprocessor::read_token_from_stream(){
    while(tokens.size() == 0){
        if(stream.peek_token().type == token::TokenType::Hash){
            this->process_directive();
        }else{
            tokens.emplace(tokens.end(),stream.get_token());
        }
    }
    auto current = tokens.begin();
    if(current->can_ignore){
        return pop_front(tokens);
    }
    auto macro_end = std::next(current);
    if(table->is_function(current->base.value)){
        if(macro_end == tokens.end()){
            macro_end = tokens.emplace(macro_end, stream.get_token());
        }
        if(macro_end->base.type != token::TokenType::LParen){
            //Function type macro but no left paren, we we just emit the token
            return pop_front(tokens);
        }
        while(macro_end->base.type != token::TokenType::RParen){
            macro_end++;
            if(macro_end == tokens.end()){
                macro_end = tokens.emplace(macro_end, stream.get_token());
                if(macro_end->base.type == token::TokenType::END){
                    throw lexer_error::PreprocessorError("Encountered end of token stream while processing function macro", 
                            tokens.back().base);
                }
            }
        }
        macro_end++;
    }
    expand_macros(current, macro_end);
    assert(current->can_ignore);
    return pop_front(tokens);
}

}//namespace lexer
