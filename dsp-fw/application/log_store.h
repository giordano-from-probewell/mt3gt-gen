#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "hw_types.h"

/* abstracts the storage: ram, eeprom, etc */
typedef struct {
    /* mandatory ops (return true on success) */
    bool (*read)(uint32_t addr, void* dst, uint16_t len);
    bool (*write)(uint32_t addr, const void* src, uint16_t len);
    bool (*erase_range)(uint32_t addr, uint32_t len); /* optional for RAM backend */
} log_store_if_t;
