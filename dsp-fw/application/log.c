#include "log.h"
#include <string.h>

#define LOG_MAGIC  0x474Cu
#define LOG_VER    0x0001u

static bool load_header(log_handle_t* h)
{
    log_header_disk_t d;
    if (!h->store.read(h->base_addr, &d, sizeof(d))) return false;
    if (d.magic != LOG_MAGIC || d.version != LOG_VER) return false;
    if (d.slot_bytes != LOG_SLOT_BYTES) return false;
    h->hdr = d;
    return true;
}

static bool save_header(log_handle_t* h)
{
    return h->store.write(h->base_addr, &h->hdr, sizeof(h->hdr));
}

bool log_init(log_handle_t* h, log_store_if_t store, uint32_t base_addr, uint32_t max_logs)
{
    h->store = store;
    h->base_addr = base_addr;

    if (load_header(h)) {
        /* already formatted */
        return true;
    }

    /* format region */
    h->hdr.magic      = LOG_MAGIC;
    h->hdr.version    = LOG_VER;
    h->hdr.slot_bytes = LOG_SLOT_BYTES;
    h->hdr.reserved   = 0;
    h->hdr.max_logs   = max_logs;
    h->hdr.wr_index   = 0;
    h->hdr.counter    = 0;

    /* optional erase full range */
    uint32_t total = sizeof(log_header_disk_t) + max_logs * LOG_SLOT_BYTES;
    if (h->store.erase_range) (void)h->store.erase_range(h->base_addr, total);

    return save_header(h);
}

static uint32_t slot_addr(const log_handle_t* h, uint32_t index)
{
    return h->base_addr + sizeof(log_header_disk_t) + (index * LOG_SLOT_BYTES);
}

bool log_append(log_handle_t* h, const void* data, uint16_t len)
{
    if (len > sizeof(((log_slot_disk_t*)0)->data)) len = sizeof(((log_slot_disk_t*)0)->data);

    log_slot_disk_t s;
    s.counter = h->hdr.counter + 1;
    s.len     = len;
    memset(s.data, 0, sizeof(s.data));
    memcpy(s.data, data, len);

    uint32_t addr = slot_addr(h, h->hdr.wr_index);
    if (!h->store.write(addr, &s, sizeof(s))) return false;

    h->hdr.counter  = s.counter;
    h->hdr.wr_index = (h->hdr.wr_index + 1u) % h->hdr.max_logs;
    return save_header(h);
}

/* n=1 -> last; n=2 -> last-1 ... */
bool log_get_last_n(log_handle_t* h, uint32_t n, log_slot_disk_t* out)
{
    if (n == 0 || n > h->hdr.max_logs) return false;
    uint32_t count = h->hdr.counter;
    if (count == 0) return false;

    /* compute slot index of the requested record */
    uint32_t last_idx = (h->hdr.wr_index + h->hdr.max_logs - 1u) % h->hdr.max_logs;
    uint32_t idx = last_idx;
    while (--n) {
        idx = (idx + h->hdr.max_logs - 1u) % h->hdr.max_logs;
    }

    uint32_t addr = slot_addr(h, idx);
    return h->store.read(addr, out, sizeof(*out));
}
