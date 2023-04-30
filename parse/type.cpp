#include "type.h"
#include <cassert>
#include <map>
namespace type{
namespace{
const std::map<std::string,int> conversion_rank = {{
    {"_Bool", 0},
    {"char", 1},{"signed char", 1}, {"unsigned char", 1},
    {"short int", 2},{"unsigned short int", 2},
    {"int", 3},{"unsigned int", 3},
    {"long int", 4},{"unsigned long int", 4},
    {"long long int", 5},{"unsigned long long int", 5},
}};

bool can_represent(const std::string& target, const std::string& source){
    if(target == "long long int" && source == "unsigned long int"){
        return false;
    }
    int target_rank = conversion_rank.at(target);
    int source_rank = conversion_rank.at(source);
    if(target_rank > source_rank){
        return true;
    }
    if(target == source || (target_rank == source_rank && target.find("unsigned") != std::string::npos)){
        return true;
    }
    return false;
}
}

//Converting
std::string integer_promotions(const std::string& type){
    assert(is_int(type) && "Tried to do integer promotions on non-integer");
    if(conversion_rank.at(type) > conversion_rank.at("int")){
        return type;
    }
    //check if int can represent
    if(can_represent("int", type)){
        return "int";
    }
    return "unsigned int";
}

std::string usual_arithmetic_conversions(std::string type1, std::string type2){
    if(type1 == "long double" || type2 == "long double"){
        return "long double";
    }
    if(type1 == "double" || type2 == "double"){
        return "double";
    }
    if(type1 == "float" || type2 == "float"){
        return "float";
    }
    type1 = integer_promotions(type1);
    type2 = integer_promotions(type2);
    if(type1 == type2){
        return type1;
    }
    bool signed1 = is_signed_int(type1);
    bool signed2 = is_signed_int(type2);
    int r1 = conversion_rank.at(type1);
    int r2 = conversion_rank.at(type2);
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
        if(can_represent(type1, type2)){
            return type1;
        }
        return "unsigned "+type1;
    }
    if(signed2){
        if(can_represent(type2, type1)){
            return type2;
        }
        return "unsigned "+type2;
    }
    assert(false && "Missing case for usual arithmetic conversions");
    return "";
}

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
