#ifndef _VALUE_
#define _VALUE_
#include <string>
#include "type.h"
namespace value{
class Value{
    //The name of the corresponding SSA register
    const std::string value;
    //ir type
    const type::CType type;
public:
    Value(std::string value, type::CType type) : value(value), type(type) {}
    std::string get_value() const{
        return value;
    }
    type::BasicType get_type() const{
        return std::get<type::BasicType>(type);
    }
};
} //namespace value
#endif
