#ifndef _TYPE_DERIVED_
#define _TYPE_DERIVED_
#include "type_basic.h"
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
class DerivedType{
    std::variant<std::unique_ptr<FuncType>> type;
public:
    explicit DerivedType(FuncType f); //Defined in type_func.cpp

    DerivedType(const DerivedType& other); //Defined in type.cpp
    DerivedType& operator=(const DerivedType& other);//Defined in type.cpp
    DerivedType(DerivedType&& other) = default;
    DerivedType& operator=(DerivedType&& other) = default;
    ~DerivedType() = default;

    friend bool is_compatible(const DerivedType&, const DerivedType&);
    friend std::string to_string(const DerivedType& type);

    template <typename Visitor>
    auto visit(Visitor&& v){
        return std::visit([&v](auto&& pointer){
            return std::invoke(v,*pointer);
        }
        ,type);
    }

    template <typename T>
    T get(){
        try{
            return *std::get<std::unique_ptr<T>>(type);
        }catch(std::exception& e){
            throw std::runtime_error("DerivedType holding incorrect type");
        }
    }
};
std::string to_string(const DerivedType& type);

typedef std::monostate VoidType;
typedef std::variant<VoidType, BasicType, DerivedType> CType;
CType make_type(VoidType);
CType make_type(BasicType);
CType make_type(DerivedType);

bool is_compatible(const DerivedType&, const DerivedType&); //Defined in type.cpp
bool is_compatible(const CType& , const CType&); //Defined in type/cpp
std::string to_string(const CType& type);

}
#endif
