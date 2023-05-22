#ifndef _TYPE_FUNC_
#define _TYPE_FUNC_
#include "type_basic.h"
#include "type_derived.h"
#include <variant>
#include <vector>
#include <optional>
namespace type{
class FuncType{
    struct FuncPrototype{
        std::vector<CType> param_types;
        bool variadic;
        FuncPrototype(std::vector<CType> param, bool variadic)
            : param_types(std::move(param)), variadic(variadic) {}
    };
    CType ret_type;
    std::optional<FuncPrototype> prototype;
    friend bool is_compatible(const FuncType& type1, const FuncType& type2);
public:
    FuncType(CType ret, std::vector<CType> param, bool variadic)
        : ret_type(std::move(ret)), prototype(std::make_optional<FuncPrototype>(std::move(param),variadic)) {}
    FuncType(CType ret)
        : ret_type(std::move(ret)), prototype(std::nullopt) {};
   bool has_prototype() const; 
};
bool is_compatible(const FuncType& type1, const FuncType& type2);

}//namespace type
#endif
