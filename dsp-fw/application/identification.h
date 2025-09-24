#ifndef __IDENTIFICATION_H_
#define __IDENTIFICATION_H_



typedef struct identification_data_st
{
    char probewell_part_number [16];    //32 bits
        //'9015-6200' – MODULE VGEN
        //'9015-6201' – MODULE STANDARD
        //'9015-6202' – MODULE SMART ZIF
    char serial_number[16];             //32 bits
    char fabrication_date[8];           //16 bits
    char last_verfication_date[8];      //16 bits
    int32_t comm_buffer_size;
    uint8_t my_address;
} identification_data_t;

typedef struct identification_st
{
    union identificationn_un
    {
        identification_data_t full;
        uint16_t raw[ sizeof(identification_data_t)];
    } data;
} identification_t;

#endif




/* DATA IN EEPROM -> stored to be easy reading writing
identification - 256 bytes
        char init_description[2] = 'I','D'
        int16 crc
        int16 size = 96
        int16 version = 1
    char probewell_part_number [16];    //32 bits
        //'9015-6200' – MODULE VGEN
        //'9015-6201' – MODULE STANDARD
        //'9015-6202' – MODULE SMART ZIF
    char serial_number[16];             //32 bits
    char fabrication_date[8];           //16 bits
    char last_verfication_date[8];      //16 bits
*/







