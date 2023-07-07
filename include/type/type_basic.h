#ifndef _TYPE_BASIC_
#define _TYPE_BASIC_
#include "type.h"
#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <cassert>
#include <set>
namespace type{

//Converting
BasicType usual_arithmetic_conversions(BasicType type1, BasicType type2);
BasicType integer_promotions(const BasicType& type);

//Promotions return false if rank is the same/type is unchanged
bool promote_one_rank(IType& type);
bool promote_one_rank(FType& type);

//Checking
bool is_signed_int(BasicType type);
bool is_unsigned_int(BasicType type);

int byte_size(const BasicType& type);
//Printing
std::string to_string(BasicType type);
std::string ir_type(BasicType type);
} //namespace type

#endif
