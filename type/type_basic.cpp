#include "type.h"
#include "type/type_basic.h"
#include <cassert>
#include <map>
#include <climits>
#include <limits>
#include <exception>
#include <string>
#include <iostream>
#include <utility>
#include <variant>
#include <cassert>
#include <set>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <cctype>
namespace type{

namespace{
const std::map<IType,int> conversion_rank = {{
    {IType::Bool, 0},
    {IType::Char, 1},{IType::SChar, 1}, {IType::UChar, 1},
    {IType::Short, 2},{IType::UShort, 2},
    {IType::Int, 3},{IType::UInt, 3},
    {IType::Long, 4},{IType::ULong, 4},
    {IType::LLong, 5},{IType::ULLong, 5},
}};
const std::map<std::string,IType> int_types = {{
    {"_Bool", IType::Bool},
    {"char", IType::Char},{"signed char", IType::SChar}, {"unsigned char", IType::UChar},
    {"short int", IType::Short},{"unsigned short int", IType::UShort},
    {"int", IType::Int},{"unsigned int", IType::UInt},
    {"long int", IType::Long},{"unsigned long int", IType::ULong},
    {"long long int", IType::LLong},{"unsigned long long int", IType::ULLong},
}};
const std::map<std::string,FType> float_types = {{
    {"float", FType::Float}, {"double", FType::Double}, {"long double", FType::LDouble},
}};
const std::map<std::multiset<std::string>, std::string> multiset_to_type = {{
    {{"_Bool"},"_Bool"},
    {{"char"},"char"},
    {{"signed", "char"}, "signed char"},
    {{"unsigned", "char"}, "unsigned char"},
    {{"short", "int"}, "short int"},
    {{"signed", "short"}, "short int"},
    {{"short"},"short int"},
    {{"signed", "short", "int"}, "short int"},
    {{"unsigned", "short"}, "unsigned short int"},
    {{"unsigned", "short", "int"}, "unsigned short int"},
    {{"int"},"int"},
    {{"signed"},"int"},
    {{"signed", "int"},"int"},
    {{"unsigned"},"unsigned int"},
    {{"unsigned", "int"},"unsigned int"},
    {{"long"},"long int"},
    {{"signed", "long"},"long int"},
    {{"int", "long"},"long int"},
    {{"signed", "int", "long"},"long int"},
    {{"unsigned", "long"},"unsigned long int"},
    {{"long", "long"},"long long int"},
    {{"signed", "long", "long"},"long long int"},
    {{"long", "int", "long"},"long long int"},
    {{"signed", "long", "int", "long"},"long long int"},
    {{"unsigned", "long", "int", "long"},"unsigned long long int"},
    {{"unsigned", "long", "long"},"unsigned long long int"},
    {{"float"},"float"},
    {{"double"},"double"},
    {{"long", "double"},"long double"},
}};

unsigned long long max_value(BasicType type){
    assert(std::holds_alternative<IType>(type) && "Can't take max value of non integer type");
    switch(std::get<IType>(type)){
        case IType::Bool:
            return 1ull;
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



int basic_bit_size(const FType& type){
    switch(type){
        case FType::Float:
            return 32;
        case FType::LDouble:
        case FType::Double:
            return 64;
    }
    __builtin_unreachable();
    assert(false);
}
int basic_bit_size(const IType& type){
    switch(type){
        case IType::Bool:
            return 1;
        case IType::SChar:
        case IType::Char:
        case IType::UChar:
            return std::numeric_limits<unsigned char>::digits;
        case IType::Short:
        case IType::UShort:
            return std::numeric_limits<unsigned short int>::digits;
        case IType::Int:
        case IType::UInt:
            return std::numeric_limits<unsigned int>::digits;
        case IType::Long:
        case IType::ULong:
            return std::numeric_limits<unsigned long>::digits;
        case IType::LLong:
        case IType::ULLong:
            return std::numeric_limits<unsigned long long>::digits;
    }
    __builtin_unreachable();
    assert(false && "Missing case in basic type");
}

}//namespace
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
    return "i" + std::to_string(basic_bit_size(type));
}
int byte_size(const BasicType& type){
    return std::visit([](const auto& t){return basic_bit_size(t)/8;},type);
}
std::string ir_literal(const std::string& literal_value, type::BasicType type){
    //For now we "cheat" in two ways
    //First, we use the C library functions
    //Second, we make long doubles the same as doubles
    //Together, this lets us avoid implementing arbitrary precision floats
    //And avoids dealing with target-dependent long doubles
    //Since llvm IR for long doubles is not target independent
    if(is_type<IType>(type)){
        return literal_value;
    }
    std::stringstream stream;
    double value = std::stod(literal_value);
    std::uint64_t num = 0;
    static_assert(sizeof(value) == 8);
    static_assert(std::numeric_limits<double>::is_iec559);
    std::memcpy(&num, &value, 8);
    if(std::get<type::FType>(type) == type::FType::Float){
        std::uint64_t exp = 0;
        std::memcpy(&exp, &value, 8);
        //Clear out exponent from num
        num <<= 12; 
        num >>= 12;
        //And clear significand from exp
        exp >>= 52; 
        int denormal_precision_loss = 0x381 - exp; //0x380 is equivalent to all 0s for a float
        if(denormal_precision_loss < 0){
            denormal_precision_loss = 0;
        }
        exp <<= 52; 
        //Remove precision from num to make it as precise as a float
        unsigned int sig_figs_lost = (52-23 + denormal_precision_loss);
        std::uint64_t bits_lost = (num << (64-sig_figs_lost)) >> (64 - sig_figs_lost);
        std::uint64_t rounding_mask = 1 << (sig_figs_lost - 1);
        num >>= sig_figs_lost;
        if((bits_lost & rounding_mask) != 0){
            if((bits_lost & (~rounding_mask)) != 0
                || ((num & 1u) == 1)
                ){
                //Either above 1/2 ULP, or we round to even
                num++;
            }
        }
        num <<= sig_figs_lost;
        num += exp;
    }
    stream << "0x" << std::hex << std::uppercase<<num;
    return stream.str();
}

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
            return (source == FType::Float);
    }
    __builtin_unreachable();
    assert(false);
}

IType to_unsigned(IType t){
    switch(t){
        case IType::Bool:
            return IType::Bool;
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
    __builtin_unreachable();
}

BasicType from_str_multiset(const std::multiset<std::string>& keywords){
    std::string type = "";
    try{
        type = multiset_to_type.at(keywords);
    }catch(std::out_of_range& e){
        throw std::runtime_error("Failed to convert list of keywords into valid type");
    }
    return from_str(type);
}

//Converting
BasicType integer_promotions(const BasicType& type){
    if(!std::holds_alternative<IType>(type)){
        return type;
    }
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
        case IType::Bool:
            ret_val = IType::UChar;
            break;
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
bool promote_one_rank(BasicType& type){
    return std::visit([](auto& t){return promote_one_rank(t);},type);
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
        case IType::Bool:
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
        case IType::Bool:
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
    return std::holds_alternative<IType>(type);
}
bool is_arith(BasicType type){
    return is_int(type) || is_float(type);
}

std::string to_string(IType type){
    switch(type){
        case IType::Bool:
            return "_Bool";
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
}//namespace type
