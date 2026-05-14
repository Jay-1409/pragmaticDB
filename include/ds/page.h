#pragma once

#include <cstddef>
#include <cstdint>

using page_id_t = int32_t;
using frame_id_t = int32_t;

constexpr page_id_t INVALID_PAGE_ID = -1;
constexpr std::size_t PAGE_SIZE = 4096;

struct Page {
    page_id_t page_id = INVALID_PAGE_ID;
    int pin_count = 0;
    bool is_dirty = false;
    char data[PAGE_SIZE] = {};
};
