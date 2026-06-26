#pragma once
#include <iostream>
#include <any>
#include "type/type_id.h"

class DsFactory {
    public:
        DsFactory();
        void test();    
        std::any get_structure(TypeId type_id);
};

//**
// Following factory design pater for data structure creation 
// 
// 
// This class will be responsible for returning the type of data's data structure
// 
// will be called by the value class */

// TODO:
// This should be the base class, for all factorys, or probably this should be the class that is called for all facrtorys
// Maybe inheritence can be added
// 
// 