#include "type.h"
namespace type{
//Converting
std::string integer_promotions(const std::string& type);

//Checking
bool is_signed_int(const std::string& type){
    return type == "signed char" ||
        type == "short int" ||
        type == "int" ||
        type == "long int" ||
        type == "long long int";
}
bool is_unsigned_int(const std::string& type){
    return type == "unsigned char" ||
        type == "unsigned short int" ||
        type == "unsigned int" ||
        type == "unsigned long int" ||
        type == "unsigned long long int";
}
bool is_float(const std::string& type){
    return type == "float" ||
        type == "double" ||
        type == "long double";
}
}
