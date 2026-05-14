#include "type/value.h"
#include "type/type_id.h"
#include "factory/data_structure_factory.h"
#include <iostream>

Value::Value(TypeId type_id) : type_id_(type_id) {
    DsFactory factory;
    data_ = factory.get_structure(type_id);
}
void Value::test() {
    std::cout << "Value is responding" << std::endl;
}

