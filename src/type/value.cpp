#include "type/value.h"
#include "type/type_id.h"
#include "factory/data_structure_factory.h"
#include <iostream>
#include <cstring> 

Value::Value(TypeId type_id) : type_id_(type_id) {
    DsFactory factory;
    data_ = factory.get_structure(type_id);
}
void Value::test() {
    std::cout << "Value is responding" << std::endl;
}
void Value::SerializeToChar(char *data) const {
    if (type_id_ == TypeId::INTEGER) {
        int32_t val = std::any_cast<int32_t>(data_);
        std::memcpy(data, &val, sizeof(int32_t));
    } 
    else if (type_id_ == TypeId::BOOLEAN) {
        int8_t val = std::any_cast<int8_t>(data_);
        std::memcpy(data, &val, sizeof(int8_t));
    }
}

void Value::DeserializeFromChar(const char *data, TypeId type_id) {
    type_id_ = type_id;

    if (type_id == TypeId::INTEGER) {
        int32_t val;
        std::memcpy(&val, data, sizeof(int32_t));
        data_ = std::make_any<int32_t>(val);
    } else if (type_id == TypeId::BOOLEAN) {
        int8_t val;
        std::memcpy(&val, data, sizeof(int8_t));
        data_ = std::make_any<int8_t>(val);
    }
}
