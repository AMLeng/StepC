#include "type.h"
#include <cassert>
#include <map>
#include <climits>
#include <limits>
namespace type{

namespace{
const std::map<IType,int> conversion_rank = {{
    {IType::Char, 1},{IType::SChar, 1}, {IType::UChar, 1},
    {IType::Short, 2},{IType::UShort, 2},
    {IType::Int, 3},{IType::UInt, 3},
    {IType::Long, 4},{IType::ULong, 4},
    {IType::LLong, 5},{IType::ULLong, 5},
}};
const std::map<std::string,IType> int_types = {{
    {"char", IType::Char},{"signed char", IType::SChar}, {"unsigned char", IType::UChar},
    {"short int", IType::Short},{"unsigned short int", IType::UShort},
    {"int", IType::Int},{"unsigned int", IType::UInt},
    {"long int", IType::Long},{"unsigned long int", IType::ULong},
    {"long long int", IType::LLong},{"unsigned long long int", IType::ULLong},
}};
const std::map<std::string,FType> float_types = {{
    {"float", FType::Float}, {"double", FType::Double}, {"long double", FType::LDouble},
}};

unsigned long long max_value(BasicType type){
    assert(std::holds_alternative<IType>(type) && "Can't take max value of non integer type");
    switch(std::get<IType>(type)){
        case IType::SChar:
            return SCHAR_MAX;
        case IType::Char:
            return CHAR_MAX;
        case IType::UChar:
            return UCHAR_MAX;
        case IType::Short:
            return SHRT_MAX;
        case IType::UShort:
            return USHRT_MAX;
        case IType::Int:
            return INT_MAX;
        case IType::UInt:
            return UINT_MAX;
        case IType::Long:
            return LONG_MAX;
        case IType::ULong:
            return ULONG_MAX;
        case IType::LLong:
            return LLONG_MAX;
        case IType::ULLong:
            return ULLONG_MAX;
    }
    __builtin_unreachable();
    assert(false);
}



std::string ir_type(FType type){
    switch(type){
        case FType::Float:
            return "float";
        case FType::LDouble:
//We cheat here since other long doubles are target dependent
//And the standard doesn't require long double to be distinct
        case FType::Double:
            return "double"; 
    }
    __builtin_unreachable();
    assert(false);
}
std::string ir_type(IType type){
    int bits = 0;
    switch(type){
        case IType::SChar:
        case IType::Char:
        case IType::UChar:
            bits = std::numeric_limits<unsigned short>::digits;
            break;
        case IType::Short:
        case IType::UShort:
            bits = std::numeric_limits<unsigned short int>::digits;
            break;
        case IType::Int:
        case IType::UInt:
            bits = std::numeric_limits<unsigned int>::digits;
            break;
        case IType::Long:
        case IType::ULong:
            bits = std::numeric_limits<unsigned long>::digits;
            break;
        case IType::LLong:
        case IType::ULLong:
            bits = std::numeric_limits<unsigned long long>::digits;
            break;
    }
    return "i" + std::to_string(bits);
}

}//namespace

BasicType make_basic(IType type){
    return std::variant<IType,FType>(type);
}
BasicType make_basic(FType type){
    return std::variant<IType,FType>(type);
}

bool can_represent(IType type, unsigned long long  value){
    return value <= max_value(type);
}
bool can_represent(IType target, IType source){
    return can_represent(target, max_value(source));
}
bool can_represent(FType target, FType source){
    switch(target){
        case FType::LDouble:
            return true;
        case FType::Double:
            //Since LDouble and Double are the same for us
            return true;
        case FType::Float:
            return (target == FType::Float);
    }
    __builtin_unreachable();
    assert(false);
}

IType to_unsigned(IType t){
    switch(t){
        case IType::SChar:
        case IType::Char:
        case IType::UChar:
            return IType::UChar;
        case IType::Short:
        case IType::UShort:
            return IType::UShort;
        case IType::Int:
        case IType::UInt:
            return IType::UInt;
        case IType::Long:
        case IType::ULong:
            return IType::ULong;
        case IType::LLong:
        case IType::ULLong:
            return IType::ULLong;
    }
    __builtin_unreachable();
    assert(false);
}


BasicType from_str(const std::string& type){
    if(int_types.find(type) != int_types.end()){
        return make_basic(int_types.at(type));
    }
    if(float_types.find(type) != float_types.end()){
        return make_basic(float_types.at(type));
    }
    assert(false && "Unknown basic type name");
}

//Converting
BasicType integer_promotions(BasicType type){
    if(conversion_rank.at(std::get<IType>(type)) > conversion_rank.at(IType::Int)){
        return type;
    }
    //check if int can represent
    if(can_represent(IType::Int, std::get<IType>(type))){
        return make_basic(IType::Int);
    }
    return make_basic(IType::UInt);
}

BasicType usual_arithmetic_conversions(BasicType type1, BasicType type2){
    auto ld = make_basic(FType::LDouble);
    if(type1 == ld || type2 == ld){
        return ld;
    }
    auto d = make_basic(FType::Double);
    if(type1 == d || type2 == d){
        return d;
    }
    auto f = make_basic(FType::Float);
    if(type1 == f || type2 == f){
        return f;
    }
    type1 = integer_promotions(type1);
    type2 = integer_promotions(type2);
    if(type1 == type2){
        return type1;
    }
    bool signed1 = is_signed_int(type1);
    bool signed2 = is_signed_int(type2);
    int r1 = conversion_rank.at(std::get<IType>(type1));
    int r2 = conversion_rank.at(std::get<IType>(type2));
    if(signed1 == signed2){
        if(r1<r2){
            return type2;
        }else{
            return type1;
        }
    }
    if(signed1 && (r1 <= r2)){
        return type2;
    }
    if(signed2 && (r2 <= r1)){
        return type1;
    }
    if(signed1){
        if(can_represent(std::get<IType>(type1), std::get<IType>(type2))){
            return type1;
        }
        return make_basic(to_unsigned(std::get<IType>(type1)));
    }
    if(signed2){
        if(can_represent(std::get<IType>(type2), std::get<IType>(type1))){
            return type2;
        }
        return make_basic(to_unsigned(std::get<IType>(type2)));
    }
    assert(false && "Missing case for usual arithmetic conversions");
    __builtin_unreachable();
}
bool promote_one_rank(IType& type){
    IType ret_val;
    switch(type){
        case IType::Char: //Char is signed by default
        case IType::SChar:
            ret_val = IType::Short;
            break;
        case IType::UChar:
            ret_val = IType::UShort;
            break;
        case IType::Short:
            ret_val = IType::Int;
            break;
        case IType::UShort:
            ret_val = IType::UInt;
            break;
        case IType::Int:
            ret_val = IType::Long;
            break;
        case IType::UInt:
            ret_val = IType::ULong;
            break;
        case IType::Long:
            ret_val = IType::LLong;
            break;
        case IType::ULong:
            ret_val = IType::ULLong;
            break;
        case IType::LLong:
            ret_val = IType::LLong;
            break;
        case IType::ULLong:
            ret_val = IType::ULLong;
            break;
    }
    if(ret_val != type){
        type = ret_val;
        return true;
    }
    return false;
}

bool promote_one_rank(FType& type){
    switch(type){
        case FType::Float:
            type = FType::Double;
            return true;
        case FType::Double:
            type = FType::LDouble;
            return true;
        case FType::LDouble:
            return false;
    }
    __builtin_unreachable();
    assert(false);
}

//Checking
bool is_signed_int(BasicType type){
    if(!std::holds_alternative<IType>(type)){
        return false;
    }
    switch(std::get<IType>(type)){
        case IType::SChar:
        case IType::Char:
        case IType::Short:
        case IType::Int:
        case IType::Long:
        case IType::LLong:
            return true;
        case IType::UChar:
        case IType::UShort:
        case IType::UInt:
        case IType::ULong:
        case IType::ULLong:
            return false;
    }
    //Annotation or g++ complains
    __builtin_unreachable();
    assert(false);
}
bool is_unsigned_int(BasicType type){
    if(!std::holds_alternative<IType>(type)){
        return false;
    }
    switch(std::get<IType>(type)){
        case IType::SChar:
        case IType::Char:
        case IType::Short:
        case IType::Int:
        case IType::Long:
        case IType::LLong:
            return false;
        case IType::UChar:
        case IType::UShort:
        case IType::UInt:
        case IType::ULong:
        case IType::ULLong:
            return true;
    }
    //Annotation or g++ complains
    __builtin_unreachable();
    assert(false);
}
bool is_float(BasicType type){
    return std::holds_alternative<FType>(type);
}
bool is_int(BasicType type){
    return is_signed_int(type) || is_unsigned_int(type);
}
bool is_arith(BasicType type){
    return is_int(type) || is_float(type);
}
bool is_scalar(BasicType type){
    return /*is_pointer(type) ||*/ is_arith(type);
}

std::string to_string(IType type){
    switch(type){
        case IType::SChar:
            return "signed char";
        case IType::Char:
            return "char";
        case IType::UChar:
            return "unsigned char";
        case IType::Short:
            return "short int";
        case IType::UShort:
            return "unsigned short int";
        case IType::Int:
            return "int";
        case IType::UInt:
            return "unsigned int";
        case IType::Long:
            return "long int";
        case IType::ULong:
            return "unsigned long int";
        case IType::LLong:
            return "long long int";
        case IType::ULLong:
            return "unsigned long long int";
    }
    __builtin_unreachable();
    assert(false);
}
std::string to_string(FType type){
    switch(type){
        case FType::Float:
            return "float";
        case FType::Double:
            return "double";
        case FType::LDouble:
            return "long double";
    }
    __builtin_unreachable();
    assert(false);
}

std::string to_string(BasicType type){
    return std::visit([](auto t){return to_string(t);},type);
}
std::string ir_type(BasicType type){
    return std::visit([](auto t){return ir_type(t);},type);
}
}//namespace type
