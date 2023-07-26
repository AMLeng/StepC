#ifndef _TYPE_
#define _TYPE_
#include <variant>
#include <cassert>
#include <set>
#include <map>
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
class ArrayType;
class PointerType;
class StructType;
class UnionType;
class EnumType;
class DerivedType;
typedef std::variant<VoidType, BasicType, DerivedType> CType;

typedef std::variant<std::unique_ptr<FuncType>, std::unique_ptr<PointerType>, 
    std::unique_ptr<StructType>, std::unique_ptr<UnionType>> DerivedPointers;
class DerivedType{
    DerivedPointers type;
public:
    DerivedType(FuncType f); //Defined in type_func.cpp
    DerivedType(PointerType f); //Defined in type_pointer.cpp
    DerivedType(ArrayType f); //Defined in type_array.cpp
    DerivedType(StructType f); //Defined in type_struct.cpp
    DerivedType(UnionType f); //Defined in type_struct.cpp

    DerivedType(const DerivedType& other); 
    DerivedType& operator=(const DerivedType& other);
    DerivedType(DerivedType&& other) = default;
    DerivedType& operator=(DerivedType&& other) = default;
    ~DerivedType() = default;

    bool operator==(const DerivedType& other) const;
    bool operator!=(const DerivedType& other) const;
    friend bool is_compatible(const DerivedType&, const DerivedType&);
    friend std::string to_string(const DerivedType& type);

    template <typename ReturnType, typename Visitor>
    ReturnType visit(Visitor&& v) const;

    template<typename T>
    friend T get(const CType& type);

    template<typename T>
    friend bool is_type(const CType& type);
};
class PointerType{
protected:
    CType underlying_type;
public:
    virtual std::string to_string() const;
    virtual std::string ir_type() const;
    explicit PointerType(CType t) : underlying_type(t) {}
    bool operator ==(const PointerType& other) const;
    bool operator !=(const PointerType& other) const;
    virtual std::unique_ptr<PointerType> copy() const;
    CType pointed_type() const;
    CType element_type() const;
    friend bool is_compatible(const PointerType& type1, const PointerType& type2);
    friend std::string ir_type(const PointerType& type);
    virtual ~PointerType();
};
class ArrayType : public PointerType{
    std::optional<int> allocated_size;
public:
    std::string to_string() const override;
    std::string ir_type() const override;
    ArrayType(CType t, std::optional<int> s) : PointerType(t), allocated_size(s){}
    bool operator ==(const ArrayType& other) const;
    bool operator !=(const ArrayType& other) const;
    std::unique_ptr<PointerType> copy() const override;
    void set_size(long long int size);
    long long int size() const;
    long long int align() const;
    bool is_complete() const;
    friend bool is_compatible(const ArrayType& type1, const ArrayType& type2);
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
    CType ret_type;
public:
    FuncType(CType ret, std::vector<CType> param, bool variadic);
    explicit FuncType(CType ret);
    bool operator ==(const FuncType& other) const;
    bool operator !=(const FuncType& other) const;
    std::unique_ptr<FuncType> copy() const;
    friend bool is_compatible(const FuncType& type1, const FuncType& type2);
    std::string to_string() const;
    std::string ir_type() const;
    bool has_prototype() const; 
    bool is_variadic() const; 
    std::vector<CType> param_types() const; 
    CType return_type() const; 
    bool params_match(std::vector<CType> arg_types) const;
};

struct StructType{
    bool complete;
    std::string tag;
    std::vector<CType> members;
    std::map<std::string, int> indices;
    explicit StructType(std::string tag) : tag(tag), complete(false) {}
    StructType(std::string tag, std::vector<CType> members, std::map<std::string, int> indices) :
        tag(tag), members(members), indices(indices), complete(true) {}
    std::string to_string() const;
    std::string ir_type() const;
    bool is_complete() const;
    std::unique_ptr<StructType> copy() const;
    long long int size(const std::map<std::string, type::CType>& tags) const;
    long long int align(const std::map<std::string, type::CType>& tags) const;
    bool operator ==(const StructType& other) const;
    bool operator !=(const StructType& other) const;
};
struct UnionType{
    bool complete;
    bool largest_computed;
    std::string tag;
    std::vector<CType> members;
    std::map<std::string, int> indices;
    CType largest;
    explicit UnionType(std::string tag) : tag(tag), complete(false), largest_computed(false) {}
    UnionType(std::string tag, std::vector<CType> members, std::map<std::string, int> indices);
    std::string to_string() const;
    std::string ir_type() const;
    bool is_complete() const;
    void compute_largest(const std::map<std::string, type::CType>& tags);
    std::unique_ptr<UnionType> copy() const;
    long long int size(const std::map<std::string, type::CType>& tags) const;
    long long int align(const std::map<std::string, type::CType>& tags) const;
    bool operator ==(const UnionType& other) const;
    bool operator !=(const UnionType& other) const;
};

typedef std::variant<type::StructType, type::UnionType> TagType;

std::string to_string(const CType& type);
bool is_compatible(const CType& , const CType&); //Defined in type.cpp
bool can_assign(const CType& from, const CType& to); //Defined in type.cpp
bool can_cast(const CType& from, const CType& to); //Defined in type.cpp

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
long long int size(const CType& type, const std::map<std::string, type::CType>& tags);
long long int align(const CType& type, const std::map<std::string, type::CType>& tags);
std::string ir_literal(const std::string& c_literal,BasicType type);
std::string ir_literal(const std::string& c_literal);

//bool can_represent(BasicType target, BasicType source);
bool is_complete(const CType& type);


//Everything below is template stuff for type::make_visitor to work properly
template <class... Ts> struct overloaded : Ts...{using Ts::operator()...;};
template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;


template <typename ReturnType, typename Pointer, typename Visitor>
ReturnType try_invoke(Visitor& v,Pointer p){
    if constexpr(std::is_convertible_v<decltype(std::invoke(v,*p)),ReturnType>){
        return std::invoke(v,*p);
    }else{
        std::invoke(v,*p);//Give this function a chance to throw an exception
        throw std::runtime_error("Tried to call visit helper on "+p->to_string()+" returning incompatible return type");
    }
}

template <typename ReturnType, typename Visitor>
ReturnType DerivedType::visit(Visitor&& v) const{
    if(std::holds_alternative<std::unique_ptr<PointerType>>(type)){
        auto pointer = std::get<std::unique_ptr<PointerType>>(type).get();
        if(auto p = dynamic_cast<ArrayType*>(pointer)){
            return try_invoke<ReturnType>(v,p);
        }else{
            return try_invoke<ReturnType>(v,pointer);
        }
    }else{
        return std::visit(overloaded{
            [&v](const auto& t)->ReturnType{return try_invoke<ReturnType>(v,t.get());}
        }, type);
        /*if(std::holds_alternative<std::unique_ptr<FuncType>>(type)){
            auto pointer = std::get<std::unique_ptr<FuncType>>(type).get();
            return try_invoke<ReturnType>(v, pointer);
        }
        assert(std::holds_alternative<std::unique_ptr<FuncType>>(type) && "Other derived types not yet implemented");
        auto pointer = std::get<std::unique_ptr<FuncType>>(type).get();
        return try_invoke<ReturnType>(v, pointer);*/
    }
}

template<typename ReturnType, typename...Ts>
struct type_visitor{
    overloaded<Ts...> inner_visitor;
    template <typename T>
    ReturnType operator()(const T& a){
        if constexpr(std::is_convertible_v<std::invoke_result_t<overloaded<Ts...>,T>,ReturnType>){
            return inner_visitor(a);
        }else{
            inner_visitor(a);
            throw std::runtime_error("Tried to call type visitor on lambda returning incompatible return type");
        }
    }
    ReturnType operator()(const BasicType& basic_type){
        if constexpr(std::is_convertible_v<decltype(std::visit(inner_visitor,basic_type)),ReturnType>){
            return std::visit(inner_visitor,basic_type);
        }else{
            std::visit(inner_visitor,basic_type);
            throw std::runtime_error("Tried to call basic type visitor on lambda returning incompatible return type");
        }
    }
    ReturnType operator()(const DerivedType& derived_type){
        if constexpr(std::is_convertible_v<decltype(derived_type.visit<ReturnType>(inner_visitor)),ReturnType>){
            return derived_type.visit<ReturnType>(inner_visitor);
        }else{
            throw std::runtime_error("Tried to call type visitor on "+type::to_string(derived_type)+" with function returning incompatible return type");
        }
    }
};

template<typename ReturnType, typename ...Ts>
type_visitor<ReturnType, Ts...> make_visitor(Ts... args){
    return type_visitor<ReturnType, Ts...>{args...};
}


template<typename T>
bool is_type(const CType& type){
    if constexpr(std::is_same_v<T,ArrayType>){
        try{
            return type::is_type<PointerType>(type)
                && dynamic_cast<ArrayType*>(std::get<std::unique_ptr<PointerType>>(std::get<DerivedType>(type).type).get());
        }catch(std::exception& e){
            throw std::runtime_error("Failed to check if "+to_string(type)+ " is array type");
        }
    }else{
        return std::visit(make_visitor<bool>(
            [](const auto& type){return std::is_convertible_v<std::decay_t<decltype(type)>,T>;}
        ), type);
    }
}

template<typename T>
T get(const CType& type){
    try{
        if constexpr(std::is_same_v<T,ArrayType>){
            auto pointer = std::get<std::unique_ptr<PointerType>>(std::get<DerivedType>(type).type).get();
            if(auto p = dynamic_cast<ArrayType*>(pointer)){
                return *p;
            }
        }else{
            if constexpr(std::is_convertible_v<T,DerivedType>){
                return *std::get<std::unique_ptr<T>>(std::get<DerivedType>(type).type).get();
            }else{
                return std::get<T>(type);
            }
        }
    }catch(std::exception& e){
        throw std::runtime_error("Incorrect type for type::get, cannot get "+std::string(typeid(T).name())+" from "+to_string(type)
            +"\n"+ e.what()+std::string("\n branch: ")+std::to_string(std::is_convertible_v<T,DerivedType>));
    }
    throw std::runtime_error("Incorrect type for type::get, cannot get "+std::string(typeid(T).name())+" from "+to_string(type));
}


}
#endif
