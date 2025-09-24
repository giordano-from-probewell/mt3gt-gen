#ifndef _CLI_H_
#define _CLI_H_

#include <stdbool.h>
#include <stdint.h>
#include "hw_ints.h"
#include "hw_types.h"
#include "generic_definitions.h"

#define CLI_TEXT_BOX_SIZE 15*76

typedef enum states_cli_test_en
{
    STATE_CLI_MENU_INIT = 0,
    STATE_CLI_MENU,
    STATE_CLI_MENU_WAINTNG_CMD,
    STATE_CLI_DATA_1_HEADER,
    STATE_CLI_DATA_1_WAINTNG_CMD,
    STATE_CLI_DATA_2_HEADER,
    STATE_CLI_DATA_2_WAINTNG_CMD,
    STATE_CLI_MENU_ENDING,
    STATE_CLI_MENU_END,
    STATE_CLI_MENU_ERROR = -1,
} states_cli_test_t;

typedef enum cli_states_en
{
    STATE_CLI_INIT             = 0,
    STATE_CLI_FIND                ,
    STATE_CLI_CONFIG_MODE         ,
    STATE_CLI_LOG_MODE            ,
    STATE_CLI_HANDLER             ,
    STATE_CLI_DATA_NEW            ,
    STATE_CLI_ERROR            =-1,

} cli_states_t;

typedef struct
{
    char buffer[CLI_TEXT_BOX_SIZE];
    int current_index;
    int size;
} cli_transmission_t;

//struct cli_menu_st;
//typedef struct cli_menu_st cli_menu_t;

typedef generic_status_t (*cli_test_handler_t)(const my_time_t time_actual, void *cli_menu, int16_t rcv);

typedef struct cli_menu_st {
    char * initialized;
    cli_test_handler_t handler;
    int16_t menu_key;
    struct {
        states_cli_test_t state;
        generic_status_t status;
    } machine;
    my_time_t scheduling;
    cli_transmission_t *tx;
} cli_menu_t;

typedef struct cli_st {
    const char *initialized;
    struct {
        cli_states_t state;
        generic_status_t status;
    } machine;

    cli_menu_t *map;
    uint16_t map_count;
    uint16_t map_idx;
    cli_transmission_t *tx;
    my_time_t scheduling;
} cli_t;

extern cli_transmission_t _cli_tx;
extern cli_menu_t cli_map[];

extern cli_t cli;

generic_status_t cli_init(void);
generic_status_t cli_processing(const my_time_t time_actual);

int16_t update_display_for_data(char *txt);
int16_t update_display_for_menu(char *txt);
int16_t update_display_for_header(char *txt);
int16_t update_display_for_log(char *txt);
int16_t update_display_return(char *txt, int pos);
int16_t update_display_restore_position(char *txt);
int16_t update_display_save_position(char *txt);
int16_t update_display_move_up(char *txt, int16_t n);


void cli_update_display_raw(char *txt);





#endif
