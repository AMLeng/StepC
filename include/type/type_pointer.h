#ifndef _TYPE_POINT_
#define _TYPE_POINT_
#include "type.h"
#include "type/type_derived.h"
#include <variant>
#include <vector>
#include <optional>
namespace type{
bool is_compatible(const PointerType& type1, const PointerType& type2);
std::string to_string(const PointerType& type);
std::string ir_type(const PointerType& type);
bool can_assign(const PointerType& right, const PointerType& left);

}//namespace type
#endif
