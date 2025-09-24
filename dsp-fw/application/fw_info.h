#ifndef __FW_INFO_H_
#define __FW_INFO_H_

typedef struct fw_data_st //  (in flash)
{
   int32_t fw_crc;
   char version[10]; //"00.01dev"
   char compilation_date[12]; //"Feb 15 2024\0"
   char compilation_time[9]; //"12:53:45\0"
   char cpu; //1 or 2
} fw_data_t;

typedef struct fw_st
{
    union union_fw_un
    {
        fw_data_t full;
        uint16_t raw[sizeof(fw_data_t)];
    } data;

} fw_t;

#endif


