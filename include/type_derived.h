#ifndef _TYPE_DERIVED_
#define _TYPE_DERIVED_
#include "type_basic.h"
#include "type.h"
#include <variant>
#include <memory>
#include <functional>
namespace type{
class FuncType;
//class ArrayType;
//class StructType;
//class UnionType;
//class PtrType;

//Wrap a std::variant of std::unique_ptr
//Functions will use value semantics
std::string to_string(const DerivedType& type);

typedef std::monostate VoidType;
typedef std::variant<VoidType, BasicType, DerivedType> CType;
//CType make_type(VoidType);
//CType make_type(BasicType);
//CType make_type(DerivedType);

bool is_compatible(const DerivedType&, const DerivedType&); //Defined in type.cpp
bool can_convert(const CType& , const CType&); //Defined in type.cpp

}
#endif
