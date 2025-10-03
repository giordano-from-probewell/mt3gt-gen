#ifndef _CLI_H_
#define _CLI_H_

#include <stdbool.h>
#include <stdint.h>
#include "hw_ints.h"
#include "hw_types.h"
#include "generic_definitions.h"

#define CLI_TEXT_BOX_SIZE 15*80

#define CLI_LOG_POOL_BYTES (2048u)  //2k for cli messages
#define CLI_LOG_RING_SIZE 64u
#define CLI_LOG_MSG_LEN   64u


typedef enum states_cli_test_en
{
    STATE_CLI_MENU_INIT = 0,
    STATE_CLI_MENU,
    STATE_CLI_DATA_HEADER,
    STATE_CLI_DATA_WAINTNG_CMD,
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

typedef enum {
    CLI_VIEW_MENU = 0,
    CLI_VIEW_LOG  = 1,
} cli_view_mode_t;

typedef enum {
    LOG_OFF=0, LOG_ERROR=1, LOG_WARN=2, LOG_INFO=3, LOG_DEBUG=4, LOG_TRACE=5,
} cli_log_level_t;

typedef generic_status_t (*cli_test_handler_t)(const my_time_t now, void *cli_menu, int16_t rcv);

typedef struct cli_menu_st {
    char * initialized;
    cli_test_handler_t handler;
    int16_t menu_key;
    const char *title;
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
generic_status_t cli_processing(const my_time_t now);
void cli_set_view(cli_view_mode_t v);
void cli_set_log_level(cli_log_level_t lvl);

//ANSI utilities
int16_t update_display_for_data(char *txt);
int16_t update_display_for_menu(char *txt);
int16_t update_display_for_header(char *txt);
int16_t update_display_for_log(char *txt);
int16_t update_display_return(char *txt, int pos);
int16_t update_display_restore_position(char *txt);
int16_t update_display_save_position(char *txt);
int16_t update_display_move_up(char *txt, int16_t n);
int16_t update_display_clear_screen(char *txt);                 // ESC[2J ESC[H
int16_t update_display_move_to(char *txt, uint16_t row, uint16_t col); // ESC[row;colH
int16_t update_display_move_down(char *txt, int16_t n);
int16_t update_display_clear_line(char *txt); // ESC[2K + CR

int16_t update_display_show_cursor(char *txt);
void cli_update_display_raw(char *txt);
void print_line(char *buf, int *pn, const char *fmt, ...);

void cli_logf(cli_log_level_t lvl, const char *fmt, ...);
// Tail of log in the panel: N = lines to show (0 = off)
void cli_set_panel_log_lines(uint8_t n);
// render last N lines do log without consume of the ring ring (just for panel)
int cli_render_tail_log(char* out, int max);


#define CLI_LOGE(...)  cli_logf(LOG_ERROR, __VA_ARGS__)
#define CLI_LOGW(...)  cli_logf(LOG_WARN , __VA_ARGS__)
#define CLI_LOGI(...)  cli_logf(LOG_INFO , __VA_ARGS__)
#define CLI_LOGD(...)  cli_logf(LOG_DEBUG, __VA_ARGS__)
#define CLI_LOGT(...)  cli_logf(LOG_TRACE, __VA_ARGS__)

#endif
