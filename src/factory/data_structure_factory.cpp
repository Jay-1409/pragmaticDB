#include "factory/data_structure_factory.h"
#include <iostream>
#include <stdexcept>
#include <cstdint>

DsFactory::DsFactory() {}

void DsFactory::test() {
    std::cout << "Data structure factory is responding" << std::endl;
}  
std::any DsFactory::get_structure(TypeId type_id) {
    switch (type_id) {
        case TypeId::INTEGER:
            return std::make_any<int32_t>(0);
        case TypeId::BOOLEAN:
            return std::make_any<int8_t>(0);
        default:
            return {}; // INVALID, returns empty std::any
    }
} 