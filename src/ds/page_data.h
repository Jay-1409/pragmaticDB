#include<cstdint>
/** It is essentially the slot, it stores the offset after which the data will start for this slot and the length of the data */
struct PageData {
    uint16_t offset;
    uint16_t length;
};