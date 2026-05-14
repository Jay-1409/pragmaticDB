#pragma once
#include "ds/page.h"
#include "ds/slot.h"
struct RecordId {
    page_id_t page_id;
    uint16_t slot_id;
};
