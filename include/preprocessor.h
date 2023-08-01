#pragma once
#include "token.h"
#include "token_stream.h"
#include <string>
#include <unordered_set>
#include <list>
namespace lexer{
struct EnhancedToken{
    explicit EnhancedToken(token::Token t) : base(t), disabled(), colored(false) {}
    token::Token base;
    std::unordered_set<std::string> disabled;
    bool colored;
};
class Preprocessor : public TokenStream{
    token::Token read_token_from_stream() override;
    std::list<EnhancedToken> tokens;
    TokenStream& stream;
public:
    Preprocessor(TokenStream& s) : stream(s) {}
};

}
