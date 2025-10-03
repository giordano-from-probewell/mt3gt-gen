#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "log_store.h"

#ifndef LOG_SLOT_BYTES
#define LOG_SLOT_BYTES 32u  /* 32 bytes per log record in storage (EEPROM-friendly) */
#endif

/* on-storage header (compact, little endian) */
typedef struct /*__attribute__((packed))*/ {
    uint16_t magic;          /* 'L' 'G' -> 0x474C */
    uint16_t version;        /* 0x0001 */
    uint16_t slot_bytes;     /* 32 default */
    uint16_t reserved;       /* pad */
    uint32_t max_logs;       /* how many slots */
    uint32_t wr_index;       /* next slot to write */
    uint32_t counter;        /* monotonically increasing per append */
} log_header_disk_t;

/* in-RAM handle */
typedef struct {
    log_store_if_t store;
    uint32_t base_addr;      /* base address in storage */
    log_header_disk_t hdr;   /* cached header */
} log_handle_t;

/* fixed 32-byte slot: [u32 counter][u16 len][24 bytes payload] */
typedef struct /*__attribute__((packed))*/ {
    uint32_t counter;
    uint16_t len;            /* <= 24 */
    uint8_t  data[24];
} log_slot_disk_t;

/* API */
bool log_init(log_handle_t* h, log_store_if_t store, uint32_t base_addr, uint32_t max_logs);
bool log_append(log_handle_t* h, const void* data, uint16_t len);
bool log_get_last_n(log_handle_t* h, uint32_t n, log_slot_disk_t* out); /* simplest iterator: get Nth from tail (n=1 last) */
