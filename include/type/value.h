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

        TypeId GetTypeId() const { return type_id_; }

        bool CompareEquals(const Value& other) const;
        bool CompareLessThan(const Value& other) const;
        bool CompareGreaterThan(const Value& other) const;
        bool CompareLessThanOrEqual(const Value& other) const;
        bool CompareGreaterThanOrEqual(const Value& other) const;
        bool CompareNotEqual(const Value& other) const;
    private:
        TypeId type_id_;
        std::any data_; 
};

//**
// This class is responsible for the data type or actual data representation */