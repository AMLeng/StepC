#ifndef _VALUE_
#define _VALUE_
#include <string>
#include "type.h"
namespace value{
class Value{
    //The name of the corresponding SSA register, with global or local indicator
    const std::string value;
    const type::CType type;
public:
    Value(std::string value, type::CType type) : value(value), type(type) {}
    std::string get_value() const{
        return value;
    }
    std::string get_value_name() const{
        return value.substr(1);
    }
    type::BasicType get_type() const{
        return std::get<type::BasicType>(type);
    }
};
} //namespace value
#endif
