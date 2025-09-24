#ifndef __LOG_H_
#define __LOG_H_


#define EACH_LOG_SIZE 30 //C2000 it means 64 bytes or 32 words, in EEPROM it means 32 bytes

typedef struct log_header_st
{
    int16_t i2c_addr;
    int16_t each_log_size;
    int32_t initial_addr;
    int32_t max_logs;
    int32_t last_log_addr;
    int32_t counter;
} log_header_t;

typedef struct log_st
{
    union log_un
    {
        log_header_t full;
        uint16_t raw[ sizeof(log_header_t) ];
    } header;
    int32_t log_num;
    char log[EACH_LOG_SIZE];
} log_t;

#endif


/* EEPROM
log - 32 + n*32 bytes
    char init_log[2] = 'L',{'B'|'F'|'E'}
    int16 crc
    int16 header_size = 14
    int16 version = 1
    int16 num_max_logs
    int16 last_log_addr
    int16 reserved (padding) = 0x00
    uint32 {log_counter|boot_counter}
    log[n]
        int16 log_num   //2 bytes
        char log[30]    // 30 bytes
 */
