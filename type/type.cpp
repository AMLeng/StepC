#include "type.h"
namespace type{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{


} //namespace

bool is_compatible(const DerivedType& type1, const DerivedType& type2){
    return std::visit(overloaded{
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<FuncType>& type1){
            if(!std::holds_alternative<std::unique_ptr<FuncType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<FuncType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return is_compatible(*type1, *std::get<std::unique_ptr<FuncType>>(type2));
            },
    }, type1.type);
}

bool is_compatible(const CType& type1, const CType& type2){
    return std::visit(overloaded{
        [&type2](VoidType){return std::holds_alternative<VoidType>(type2);},
        [&type2](BasicType type1){return std::holds_alternative<BasicType>(type2)
                            && type1 == std::get<BasicType>(type2);},
        [&type2](const DerivedType& type1){return std::holds_alternative<DerivedType>(type2) 
                            && is_compatible(type1, std::get<DerivedType>(type2));},
    }, type1);
}

} //namespace type
