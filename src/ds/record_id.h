#pragma once
#include "page.h"
#include "slot.h"
struct RecordId {
    page_id_t page_id;
    uint16_t slot_id;
};
