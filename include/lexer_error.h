#ifndef _LEXER_ERRORS_
#define _LEXER_ERRORS_
#include <string>
#include <string_view>
#include <exception>
#include "lexer.h"
namespace lexer_error{
class LexError : public std::exception{
    private:
        std::string error_str;
    public:
        LexError(std::string_view what_arg,const std::string& value, char c, std::pair<int, int> tok_start){
            error_str = ("Lexer error starting at line " + 
                std::to_string(tok_start.first) +" column "+std::to_string(tok_start.second) +
                " on token " + value + c + ": ").append(what_arg);
        }
        LexError(std::string_view what_arg, const token::Token& tok){
            error_str = ("Lexer error on " + tok.to_string()).append(what_arg);
        }
        const char* what() const noexcept override{
            return error_str.c_str();
        }
};
class PreprocessorError : public LexError{
    public:
        PreprocessorError(std::string_view what_arg,token::Token tok)
            : LexError(what_arg, tok) {}
};

class NotImplemented : public LexError{
    public:
        NotImplemented(std::string_view what_arg,const std::string& value, char c, std::pair<int, int> tok_start)
            : LexError(what_arg, value, c, tok_start) {}
};
class UnknownInput : public LexError{
    public:
        UnknownInput(std::string_view what_arg,const std::string& value, char c, std::pair<int, int> tok_start)
            : LexError(what_arg, value, c, tok_start) {}
};

class InvalidLiteral : public LexError{
    public:
        InvalidLiteral(std::string_view what_arg,const std::string& value, char c, std::pair<int, int> tok_start)
            : LexError(what_arg, value, c, tok_start) {}
};

} //namespace lexer_error
#endif
