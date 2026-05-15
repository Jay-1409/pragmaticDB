#pragma once
#include <any>
#include <cstdint>
#include "type/type_id.h"

class Value {
    public:
        Value(TypeId type_id);
        void SerializeToChar(char *data) const;
        void DeserializeFromChar(const char *data, TypeId type_id);
        template <typename T>
        T Get() const {
            return std::any_cast<T>(data_);
        }
        template <typename T>
        void Set(T val) {
            data_ = std::make_any<T>(val);
        }        
        void test();
    private:
        TypeId type_id_;
        std::any data_; 
};

//**
// This class is responsible for the data type or actual data representation */