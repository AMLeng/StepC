#ifndef _TYPE_
#define _TYPE_
#include <string>
#include <utility>
namespace type{
//Converting
std::string integer_promotions(const std::string& type);
std::string usual_arithmetic_conversions(std::string type1, std::string type2);

//Checking
bool is_signed_int(const std::string& type);
bool is_unsigned_int(const std::string& type);
bool is_float(const std::string& type);

inline bool is_int(const std::string& type){
    return is_signed_int(type) || is_unsigned_int(type);
}
inline bool is_arith(const std::string& type){
    return is_int(type) || is_float(type);
}
inline bool is_scalar(const std::string& type){
    return /*is_pointer(type) ||*/ is_arith(type);
}
}
#endif
