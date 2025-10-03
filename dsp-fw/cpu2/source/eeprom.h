/* eeprom.h */
#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <stdbool.h>
#include "F2837xD_device.h"
#include "hw_types.h"
#include "my_i2c.h"

// Blocking EEPROM API (existing)
#define EEPROM_CTRL_ERROR_COUNTER_GET   1
#define EEPROM_CTRL_ERROR_COUNTER_CLEAR 2
#define EEPROM_CTRL_MEMORY_INFO         3
#define EEPROM_SEEK_SET                 0
#define EEPROM_SEEK_CUR                 1
#define EEPROM_SEEK_END                 2

typedef struct {
    volatile struct I2C_REGS *i2c_regs;
    uint8_t  i2c_address;
    uint16_t memory_size;
    uint16_t page_size;
    uint16_t address_pointer;
    uint16_t error_counter;
} eeprom_info_t;

int  eeprom_initiate(eeprom_info_t *eeprom);
int  eeprom_terminate(eeprom_info_t *eeprom);
int  eeprom_read(eeprom_info_t *eeprom, char *buf, unsigned count);
int  eeprom_write(eeprom_info_t *eeprom, const char *buf, unsigned count);
long eeprom_seek(eeprom_info_t *eeprom, long offset, int origin);
int  eeprom_ctrl(eeprom_info_t *eeprom, int cmd, void *argp);

// Non-blocking EEPROM API
#define EEPROM_NB_MAX_PAGE_SIZE 256

typedef enum {
    EEPROM_NB_IDLE,
    EEPROM_NB_BUSY,
    EEPROM_NB_DONE,
    EEPROM_NB_ERROR
} eeprom_nb_state_t;

typedef struct {
    eeprom_info_t   *info;
    i2c_context_t   *i2c_ctx;          // shared I2C context
    uint8_t          write_buf[EEPROM_NB_MAX_PAGE_SIZE + 2];
    char            *read_buf;
    uint16_t         xfer_len;
    eeprom_nb_state_t state;
    int16_t          result;
} eeprom_nb_handle_t;

// Initialize non-blocking EEPROM handle with shared I2C context
int  eeprom_nb_init(eeprom_nb_handle_t *h, eeprom_info_t *info, i2c_context_t *i2c_ctx);
// Start non-blocking read/write
int  eeprom_read_nb_start(eeprom_nb_handle_t *h, char *buf, unsigned count);
int  eeprom_write_nb_start(eeprom_nb_handle_t *h, const char *buf, unsigned count);
// Process state machine (call in main loop)
void eeprom_nb_process(eeprom_nb_handle_t *h);
// Get result count or error (<0)
int  eeprom_nb_result(eeprom_nb_handle_t *h);
int eeprom_nb_selftest(eeprom_info_t *info, i2c_context_t *i2c_ctx);
int eeprom_sync_selftest(void) ;

#endif /* EEPROM_H */
