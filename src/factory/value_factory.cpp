#include "factory/value_factory.h"
#include <stdexcept>

Value ValueFactory::FromString(const std::string& raw, TypeId type) {
    Value val(type);

    switch (type) {
        case TypeId::INTEGER:
            val.Set<int32_t>(std::stoi(raw));
            break;

        case TypeId::BOOLEAN: {
            std::string upper = raw;
            for (auto& c : upper) c = toupper(c);
            if (upper != "TRUE" && upper != "FALSE") {
                throw std::runtime_error(
                    "ValueFactory::FromString: expected 'true' or 'false', got '" + raw + "'");
            }
            val.Set<int8_t>(upper == "TRUE" ? 1 : 0);
            break;
        }

        default:
            throw std::runtime_error(
                "ValueFactory::FromString: unsupported TypeId for raw value '" + raw + "'");
    }

    return val;
}

std::string ValueFactory::ToString(const Value& val, TypeId type) {
    switch (type) {
        case TypeId::INTEGER:
            return std::to_string(val.Get<int32_t>());

        case TypeId::BOOLEAN:
            return val.Get<int8_t>() ? "true" : "false";

        default:
            throw std::runtime_error(
                "ValueFactory::ToString: unsupported TypeId");
    }
}
