
#ifndef __STATUS_H_
#define __STATUS_H_



typedef struct dev_status_data_st
{
    int32_t boot_counter;
} dev_status_data_t;

typedef struct dev_status_st
{
    union dev_status_un
    {
        dev_status_data_t full;
        uint16_t raw[ sizeof(dev_status_data_t) ];
    } data;
} dev_status_t;

#endif
