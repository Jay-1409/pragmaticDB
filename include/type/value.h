#pragma once
#include <any>
#include <cstdint>
#include "type/type_id.h"

class Value {
    public:
        Value(TypeId type_id);
        
        void test();
        
    private:
        TypeId type_id_;
        std::any data_; 
};

//**
// This class is responsible for the data type or actual data representation */