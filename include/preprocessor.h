#pragma once
#include "token.h"
#include "token_stream.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <memory>
namespace lexer{
struct EnhancedToken{
    explicit EnhancedToken(token::Token t) : base(t), disabled(), can_ignore(t.type != token::TokenType::Identifier) {}
    EnhancedToken(std::pair<token::TokenType,std::string> replacement_data, const EnhancedToken& token_to_expand);
    token::Token base;
    std::unordered_set<std::string> disabled;
    bool can_ignore;
};
class Preprocessor : public TokenStream{
    struct MacroTable;
    TokenStream& stream;
    std::list<EnhancedToken> tokens;
    std::unique_ptr<MacroTable> table;
    void process_directive();
    void expand_macros(decltype(tokens.begin()) start, decltype(tokens.end()) end);
    token::Token read_token_from_stream() override;
public:
    Preprocessor(TokenStream& s);
    ~Preprocessor();
};

}
