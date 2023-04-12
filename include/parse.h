#ifndef _PARSE_
#define _PARSE_
#include "lexer.h"
#include "ast.h"
#include <memory>
namespace parse{

std::unique_ptr<ast::Program> construct_ast(lexer::Lexer& l);

} //namespace parse
#endif
