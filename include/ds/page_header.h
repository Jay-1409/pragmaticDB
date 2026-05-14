#pragma once
#include <cstdint>
#include "ds/page.h"
struct PageHeader {
    page_id_t page_id;
    uint16_t free_space_offset;
    uint16_t slot_count;
};