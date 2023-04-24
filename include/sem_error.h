#ifndef _SEM_ERROR_
#define _SEM_ERROR_
#include <string>
#include <exception>
#include "lexer.h"
namespace sem_error{
class SemError : public std::exception{
    private:
        std::string error_str;
    public:
        SemError(std::string_view what_arg, const token::Token& tok){
            error_str = ("Semantic error on token " + tok.value + 
            " at line "+std::to_string(tok.loc.start_line)+" and column "+std::to_string(tok.loc.start_col)
            +": ").append(what_arg);
        }
        const char* what() const noexcept override{
            return error_str.c_str();
        }
};

class TypeError : public SemError{
    public:
        TypeError(std::string_view what_arg,token::Token tok)
            : SemError(what_arg, tok) {}
};

class UnknownError : public SemError{
    public:
        UnknownError(std::string_view what_arg,token::Token tok)
            : SemError(what_arg, tok) {}
};

} //namespace sem_error
#endif
