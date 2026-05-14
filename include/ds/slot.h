#pragma once
#include <cstdint>

/** Slot entry storing tuple offset and length. */
struct Slot {
    uint16_t offset;
    uint16_t length;
};
