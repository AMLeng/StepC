#ifndef _TYPE_DERIVED_
#define _TYPE_DERIVED_
#include "type_basic.h"
#include <variant>
#include <memory>
namespace type{
class FuncType;
//class ArrayType;
//class StructType;
//class UnionType;
//class PtrType;

//Wrap a std::variant of std::unique_ptr
//Functions will use value semantics
class DerivedType{
    std::variant<std::unique_ptr<FuncType>> type;
    friend bool is_compatible(const DerivedType&, const DerivedType&);
    DerivedType(FuncType&& f);
public:
    DerivedType make_derived(FuncType&& f);
};

typedef std::monostate VoidType;
typedef std::variant<VoidType, BasicType, DerivedType> CType;

bool is_compatible(const DerivedType&, const DerivedType&); //Defined in type.cpp
bool is_compatible(const CType& , const CType&); //Defined in type/cpp

}
#endif
