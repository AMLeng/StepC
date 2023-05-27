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
    static std::string to_string(const FuncType::FuncPrototype& );
    std::optional<FuncPrototype> prototype;
    friend bool is_compatible(const FuncType& type1, const FuncType& type2);
    friend std::string to_string(const FuncType& type);
public:
    CType ret_type;
    FuncType(CType ret, std::vector<CType> param, bool variadic)
        : ret_type(std::move(ret)), prototype(std::make_optional<FuncPrototype>(std::move(param),variadic)) {}
    explicit FuncType(CType ret)
        : ret_type(std::move(ret)), prototype(std::nullopt) {assert(false && "K&R style decls not yet implemented");};
   bool has_prototype() const; 
};
std::string to_string(const FuncType& type);
bool is_compatible(const FuncType& type1, const FuncType& type2);
CType make_type(FuncType);

}//namespace type
#endif
