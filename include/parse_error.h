#ifndef _PARSE_ERROR_
#define _PARSE_ERROR_
#include <string>
#include <exception>
#include "lexer.h"
namespace parse_error{
class ParseError : public std::exception{
    private:
        std::string error_str;
    public:
        ParseError(std::string_view what_arg, const token::Token& tok){
            error_str = ("Parse error on " + tok.to_string()).append(what_arg);
        }
        const char* what() const noexcept override{
            return error_str.c_str();
        }
};

class UnknownError : public ParseError{
    public:
        UnknownError(std::string_view what_arg,token::Token tok)
            : ParseError(what_arg, tok) {}
};

} //namespace parse_error
#endif
