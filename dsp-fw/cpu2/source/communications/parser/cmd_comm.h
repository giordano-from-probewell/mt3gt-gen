#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <stdint.h>
#include "hw_types.h"
#include "my_time.h"

//TODO: verify carefully the amount of RAM available
#define COMMAND_BUFFER_SIZE 5000

enum command_comm_data_en
{
    COMMAND_PARSER_DATA__UNAVAILABLE = -1,
};

typedef enum command_comm_status_en
{
    COMMAND_PARSER_STATUS__INVALID_PARAM = -3,
    COMMAND_PARSER_STATUS__NOT_INITIALIZED = -2,
    COMMAND_PARSER_STATUS__ERROR = -1,
    COMMAND_PARSER_STATUS__NOTHING = 0,
    COMMAND_PARSER_STATUS__PREPARING = 1,
    COMMAND_PARSER_STATUS__PROCESSING = 2,
    COMMAND_PARSER_STATUS__DONE = 3,
} command_comm_status_t;


typedef int16_t (*command_comm_handler_t)
        (
                uint8_t *cmd_error_category,
                uint8_t *cmd_error_code, uint16_t *cmd_number,
                uint8_t *cmd_data, uint16_t *cmd_size
        );

typedef struct command_comm_map_st
{
    command_comm_handler_t handler;
    int16_t number;
    int16_t reserved;
} command_comm_map_t;

extern command_comm_status_t command_comm_initiate(const command_comm_map_t *commands, const uint16_t count);
extern command_comm_status_t command_comm_reset(void);
extern command_comm_status_t command_comm_receive(const my_time_t time_actual, const int16_t data);
extern command_comm_status_t command_comm_process(void);
extern command_comm_status_t command_comm_transmit(int16_t *data);
extern command_comm_status_t command_comm_error(int16_t *data);

#endif
