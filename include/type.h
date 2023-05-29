#ifndef _TYPE_
#define _TYPE_
#include <variant>
#include <set>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
namespace type{
enum class IType {
    Char, SChar, UChar, 
    Short, UShort, 
    Int, UInt, 
    Long, ULong, 
    LLong, ULLong, Bool
};
enum class FType {
    Float, Double, LDouble
};
typedef std::variant<IType, FType> BasicType;
typedef std::monostate VoidType;
class FuncType;
class DerivedType;
typedef std::variant<VoidType, BasicType, DerivedType> CType;

class DerivedType{
    std::variant<std::unique_ptr<FuncType>> type;
public:
    DerivedType(FuncType f); //Defined in type_func.cpp

    DerivedType(const DerivedType& other); //Defined in type.cpp
    DerivedType& operator=(const DerivedType& other);//Defined in type.cpp
    DerivedType(DerivedType&& other) = default;
    DerivedType& operator=(DerivedType&& other) = default;
    ~DerivedType() = default;

    bool operator==(const DerivedType& other) const;
    bool operator!=(const DerivedType& other) const;
    friend bool is_compatible(const DerivedType&, const DerivedType&);
    friend std::string to_string(const DerivedType& type);
    friend bool can_convert(const DerivedType& type1, const DerivedType& type2);

    template <typename Visitor>
    auto visit(Visitor&& v) const{
        return std::visit([&v](auto&& pointer){
            return std::invoke(v,*pointer);
        }
        ,type);
    }

    template <typename T>
    T get() const{
        try{
            return *std::get<std::unique_ptr<T>>(type);
        }catch(std::exception& e){
            throw std::runtime_error("DerivedType holding incorrect type");
        }
    }
    template <typename T>
    bool holds_alternative() const{
        return std::holds_alternative<std::unique_ptr<T>>(type);
    }
};
class FuncType{
    struct FuncPrototype{
        std::vector<CType> param_types;
        bool variadic;
        FuncPrototype(std::vector<CType> param, bool variadic)
            : param_types(std::move(param)), variadic(variadic) {}
    };
    static std::string to_string(const FuncType::FuncPrototype& );
    std::optional<FuncPrototype> prototype;
public:
    bool operator ==(const FuncType& other) const;
    bool operator !=(const FuncType& other) const;
    friend bool is_compatible(const FuncType& type1, const FuncType& type2);
    friend std::string to_string(const FuncType& type);
    CType ret_type;
    FuncType(CType ret, std::vector<CType> param, bool variadic);
    explicit FuncType(CType ret);
    bool has_prototype() const; 
    bool params_match(std::vector<CType> arg_types) const;
};

std::string to_string(const CType& type);
bool is_compatible(const CType& , const CType&); //Defined in type.cpp
bool can_convert(const CType& , const CType&); //Defined in type.cpp

BasicType from_str_multiset(const std::multiset<std::string>& keywords);
BasicType usual_arithmetic_conversions(CType type1, CType type2);
BasicType integer_promotions(const CType& type);
bool is_signed_int(CType type);
bool is_unsigned_int(CType type);
bool is_float(CType type);

bool is_int(CType type);
bool is_arith(CType type);
bool is_scalar(CType type);

bool can_represent(IType type, unsigned long long int value);
bool can_represent(IType target, IType source);
bool can_represent(FType target, FType source);

std::string ir_type(const CType& type);
BasicType from_str(const std::string& type);
bool promote_one_rank(IType& type);
IType to_unsigned(IType type); 
bool can_represent(IType type, unsigned long long int value);


//bool can_represent(BasicType target, BasicType source);
//bool is_complete(CType type);

template <class... Ts> struct overloaded : Ts...{using Ts::operator()...;};
template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

template<typename ReturnType, typename...Ts>
struct type_visitor{
    overloaded<Ts...> inner_visitor;
    template <typename T>
    ReturnType operator()(const T& a){
        return inner_visitor(a);
    }
    ReturnType operator()(const BasicType& a){
        return std::visit(inner_visitor,a);
    }
    ReturnType operator()(const DerivedType& a){
        return a.visit(inner_visitor);
    }
};

template<typename ReturnType, typename ...Ts>
type_visitor<ReturnType, Ts...> make_visitor(Ts... args){
    return type_visitor<ReturnType, Ts...>{args...};
}


template<typename T>
bool is_type(const CType& type){
    return std::visit(make_visitor<bool>(
        [](const auto& type){return std::is_convertible_v<std::decay_t<decltype(type)>,T>;}
    ), type);
}


}
#endif
