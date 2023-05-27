#ifndef _TYPE_FUNC_
#define _TYPE_FUNC_
#include "type_basic.h"
#include "type_derived.h"
#include <variant>
#include <vector>
#include <optional>
namespace type{
std::string to_string(const FuncType& type);
bool is_compatible(const FuncType& type1, const FuncType& type2);
//CType make_type(FuncType);

}//namespace type
#endif
