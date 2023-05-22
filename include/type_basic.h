#ifndef _TYPE_BASIC_
#define _TYPE_BASIC_
#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <cassert>
#include <set>
namespace type{
//Basic types
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


BasicType make_basic(IType type);
BasicType make_basic(FType type);

//from_str assumes that keywords defining
//type are correctly ordered
BasicType from_str(const std::string& type);
//from_str_vec takes a vector of keywords,
//possibly out of order, and computes the type
BasicType from_str_multiset(const std::multiset<std::string>& keywords);

//Converting
BasicType usual_arithmetic_conversions(BasicType type1, BasicType type2);
BasicType integer_promotions(const BasicType& type);

//I or F specific conversions
IType to_unsigned(IType type); 

//Promotions return false if rank is the same/type is unchanged
bool promote_one_rank(IType& type);
bool promote_one_rank(FType& type);

//Checking
bool can_represent(IType type, unsigned long long int value);
bool can_represent(IType target, IType source);
bool can_represent(FType target, FType source);
bool is_signed_int(BasicType type);
bool is_unsigned_int(BasicType type);
bool is_float(BasicType type);

bool is_int(BasicType type);
bool is_arith(BasicType type);
bool is_scalar(BasicType type);

//Printing
std::string to_string(BasicType type);
std::string ir_type(BasicType type);
} //namespace type

#endif
