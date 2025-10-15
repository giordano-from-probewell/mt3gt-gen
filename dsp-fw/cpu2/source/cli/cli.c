#include "cli.h"
#include <stdarg.h>
#include "probewell_ascii.h"
#include "types.h"
#include "F28x_Project.h"
#include "string.h"
#include "sci.h"
#include "application.h"
#include "generic_definitions.h"

#include "cli_test_control1.h"
#include "cli_test.h"

const unsigned char escRed[] = {0x1B, 0x5B, '3','1', 'm'};
const unsigned char escBlue[] = {0x1B, 0x5B, '3','4', 'm'};
const unsigned char escGreen[] = {0x1B, 0x5B, '3','2', 'm'};
const unsigned char escYellow[] = {0x1B, 0x5B, '3','3', 'm'};
const unsigned char escWhite[] = {0x1B, 0x5B, '3','7', 'm'};
const unsigned char escPosHome[] = {0x1B, 0x5B, 'H'};
const unsigned char escSavePointer[] = {0x1B, 0x5B, 's'};
const unsigned char escRestorePointer[] = {0x1B, 0x5B, 'u'};
const unsigned char escInverse[] = {0x1B, 0x5B, '7', 'm'};
const unsigned char escInverseReset[] = {0x1B, 0x5B, '2', '7', 'm'};
const unsigned char escBold[] = {0x1B, 0x5B, '1', 'm'};
const unsigned char escBoldReset[] = {0x1B, 0x5B, '2', '2', 'm'};
const unsigned char Menu[] = {0x1B, 0x5B, '2', '2', 'm'};
const unsigned char escCursorInvisible[] = {0x1B, 0x5B,'?', '2', '5', 'l'};
const unsigned char escUnderline[] = {0x1B, 0x5B, '^', '[', '[', '4' ,'m'};
const unsigned char escNoCharAttributes[] = {0x1B, 0x5B, '^', '[', '[', '0' ,'m'};


#ifndef CLI_LOG_RING_SIZE
#define CLI_LOG_RING_SIZE 64u
#endif
#define CLI_LOG_MSG_LEN   64u

static bool s_log_header_done = false;
static bool s_quick_menu_printed = false;

static uint8_t  s_panel_log_lines = 0;     // 0=off; 4 recommended
static uint16_t s_clear_log_lines = 0;

static uint32_t s_quick_menu_until_ms = 0;


#define QUICK_MENU_DURATION_MS 1000


//cli sigleton
/*static*/ cli_t cli;


typedef struct {
    uint32_t t_ms;
    uint8_t  level;
    char     msg[CLI_LOG_MSG_LEN];
} cli_log_entry_t;


cli_transmission_t _cli_tx;

cli_menu_t _cli_map[] = {
  { .initialized=NULL, .tx=&_cli_tx, .handler=cli_control1_handler,  .menu_key='c', .title="Control"   },
  { .initialized=NULL, .tx=&_cli_tx, .handler=cli_test_menu_handler, .menu_key='m', .title="Menu test" },
};



static volatile cli_log_entry_t s_log_ring[CLI_LOG_RING_SIZE];
static volatile uint16_t s_log_head = 0, s_log_tail = 0;

static volatile cli_view_mode_t  s_view  = CLI_VIEW_MENU;
static volatile cli_log_level_t  s_level = LOG_INFO;


static void _draw_logo(void);
static void _log_push(uint32_t t_ms, uint8_t level, const char *msg);
static bool _log_pop(cli_log_entry_t *out);
static uint16_t _log_copy_last(uint16_t max, cli_log_entry_t* dst);
static int _print_quick_menu(char* out, int max);
static int _find_menu_by_key(int16_t key);

//
// drawLogo -
//
static void _draw_logo(void)
{
    unsigned short indexX=0;
    unsigned short indexY=0;
    char sci[50];
    unsigned char ucChar;

    SCI_writeCharBlockingNonFIFO(CLI_SERIALPORT,'\n');
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)escRed, 5);

    while(indexY<LINES)
    {
        if(indexY<LINES)
        {
            if(indexX<COLUMNS)
            {
                ucChar = logo_ascii[indexY][indexX++];
                SCI_writeCharBlockingNonFIFO(CLI_SERIALPORT,ucChar);
            }
            else
            {
                SCI_writeCharBlockingNonFIFO(CLI_SERIALPORT,'\r');
                SCI_writeCharBlockingNonFIFO(CLI_SERIALPORT,'\n');
                indexX=0;
                indexY++;
            }
        }
    }

    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)escYellow, 5);
    sprintf(sci, "COMPILATION DATE: %s %s\r\n\r\n",
            fw_info.compilation_date,fw_info.compilation_time);
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)sci, strlen(sci));
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)escWhite, 5);
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)escCursorInvisible, 6);


}




int count_lines(char *str, int n)
{
    int line_count = 0;
    int char_counter = 0;
    const char *ptr = str;

    while (*ptr != '\0' && char_counter++ < n)
    {
        if (*ptr == '\n')
        {
            line_count++;
        }
        ptr++;
    }
    if (ptr != str && *(ptr - 1) != '\n')
    {
        line_count++;
    }

    return line_count;
}

int16_t update_display_clear_screen(char *txt)
{
    return snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[2J\x1b[H");
}

int16_t update_display_move_to(char *txt, uint16_t r, uint16_t c)
{
    return snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[%u;%uH", r, c);
}

int16_t update_display_show_cursor(char *txt)
{
    return snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[?25h");
}


int16_t update_display_for_log(char *txt)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[27m\x1b[37m");
    return i;
}

int16_t update_display_for_menu(char *txt)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[7m\x1b[32m");
    return i;
}

int16_t update_display_for_header(char *txt)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[27m\x1b[32m");
    return i;
}

int16_t update_display_for_data(char *txt)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[27m\x1b[33m");
    return i;
}

int16_t update_display_move_up(char *txt, int16_t n)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[%dA", n);
    return i;
}

int16_t update_display_save_position(char *txt)
{

    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[s");
    return i;
}

int16_t update_display_restore_position(char *txt)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[u");
    return i;
}

void cli_update_display_raw(char *txt)
{
    size_t length = strlen(txt);
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)txt, length);
}

int16_t update_display_move_down(char *txt, int16_t n)
{
    return snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[%dB", n);
}
int16_t update_display_clear_line(char *txt)
{
    return snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[2K\r");
}

generic_status_t cli_init (void)
{
    volatile int status = 0;
    char aux[2];
    cli.tx = &_cli_tx;
    (cli.tx)->current_index = 0;

    //init machine
    SCI_clearInterruptStatus(CLI_SERIALPORT, SCI_INT_TXFF);
    SCI_clearInterruptStatus(CLI_SERIALPORT, SCI_INT_RXFF);

    // Initialize SCIA - GPIO must be initialized before this
    SCI_setConfig(CLI_SERIALPORT, 50000000, 115200, (SCI_CONFIG_WLEN_8| SCI_CONFIG_STOP_ONE| SCI_CONFIG_PAR_NONE));
    SCI_enableModule(CLI_SERIALPORT);
    SCI_performSoftwareReset(CLI_SERIALPORT);

    cli.map = _cli_map;
    cli.machine.state = STATE_CLI_INIT;
    cli.map_count =   sizeof(_cli_map) / sizeof(cli_menu_t);
    cli.map_idx = 0;

    // Print a Logo
    aux[0] = '\r';
    aux[1] = '\n';
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)aux, 2);
    _draw_logo();

    cli.initialized = INITIALIZED;

    return STATUS_DONE;
}



void _continue_non_blocking_transmission(const my_time_t now, cli_transmission_t  *tx)
{
    if (tx->current_index < tx->size)
    {
        if (SCI_isSpaceAvailableNonFIFO(CLI_SERIALPORT))
        {
            SCI_writeCharNonBlocking(CLI_SERIALPORT, tx->buffer[tx->current_index]);
            tx->current_index++;
            if(tx->current_index>=tx->size){
                tx->size = 0;
                tx->current_index = 0;
            }

        }
    }
}

generic_status_t cli_processing (const my_time_t now)
{
    generic_status_t status = STATUS_DONE;
    char cmd;
    int i;

    if(cli.initialized != INITIALIZED)
    {
        cli.machine.state = STATE_CLI_INIT;
    }

    if(cli.tx->size > 0)
    {
        _continue_non_blocking_transmission(now, cli.tx);
        return STATUS_PROCESSING;
    }


    cmd = -1;
    if(SCI_isDataAvailableNonFIFO(CLI_SERIALPORT))
    {
        cmd = SCI_readCharNonBlocking(CLI_SERIALPORT);
    }


    switch(cli.machine.state)
    {

    case STATE_CLI_INIT:
    {
        cli.map_idx = 0;
        cli.initialized = INITIALIZED;
        cli.scheduling = now + 100;
        cli.machine.state = STATE_CLI_LOG_MODE;
        s_log_header_done = false;
        break;
    }

    case STATE_CLI_CONFIG_MODE:
    {
        //        int n = update_display_for_menu(cli.tx->buffer);
        //        n += snprintf(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)  - n ,
        //                      "\r\n *** CONFIG MODE *** \r\n");
        //        cli.scheduling = now + 100;
        //        cli.tx->size = n;
        cli.machine.state = STATE_CLI_HANDLER;
        break;
    }

    case STATE_CLI_LOG_MODE:
    {
        // 1) If user press a level change key
        if (cmd != -1) {
                if      (cmd=='e'||cmd=='E') { s_level = LOG_ERROR;  s_log_header_done = false; }
                else if (cmd=='w'||cmd=='W') { s_level = LOG_WARN;   s_log_header_done = false; }
                else if (cmd=='i'||cmd=='I') { s_level = LOG_INFO;   s_log_header_done = false; }
                else if (cmd=='d'||cmd=='D') { s_level = LOG_DEBUG;  s_log_header_done = false; }
                else if (cmd=='t'||cmd=='T') { s_level = LOG_TRACE;  s_log_header_done = false; }
                else if (cmd=='?') {
                    s_quick_menu_until_ms = (uint32_t)now + QUICK_MENU_DURATION_MS;
                    s_quick_menu_printed = false;
                }
                else if (cmd=='l' || cmd=='L') {
                    // already in LOG mode, reprint header
                    s_log_header_done = false;
                }
                else {
                    // new menu?
                    int idx = _find_menu_by_key(cmd);
                    if (idx >= 0) {
                        cli.map_idx = (uint16_t)idx;
                        cli.machine.state = STATE_CLI_CONFIG_MODE;
                        break;
                    }
                }
            }

        // 2) If nothing, print header and/or nex log line
        if (cli.tx->size == 0) {
            if (!s_log_header_done) {
                int n = 0;
                //n += update_display_clear_screen(&(cli.tx->buffer[n]));
                n += snprintf(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)-n,
                    "\x1b[7m\x1b[32m LIVE LOG \x1b[27m\x1b[37m  level=%d  "
                    "[E]rror [W]arn [I]nfo [D]ebug [T]race   [?] menu\r\n",
                    (int)s_level);
                cli.tx->size = n; cli.tx->current_index = 0;
                s_log_header_done = true;
                break;
            }

            // 2) Banner if '?' pressed
            if (s_quick_menu_until_ms) {
                if ((uint32_t)now < s_quick_menu_until_ms) {
                    if (!s_quick_menu_printed) {
                        int n = 0;
                        n += _print_quick_menu(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)-n);
                        cli.tx->size = n; cli.tx->current_index = 0;
                        s_quick_menu_printed = true;    // não repetir
                    }
                    break;  // mantém log rodando
                } else {
                    s_quick_menu_until_ms = 0;
                    s_quick_menu_printed = false;
                }
            }

            // next log line
            cli_log_entry_t ev;
            if (_log_pop(&ev)) {
                int n = 0;
                n += snprintf(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)-n,
                              "%8lu  L%u  %s\r\n",
                              (unsigned long)ev.t_ms, (unsigned)ev.level, ev.msg);
                cli.tx->size = n; cli.tx->current_index = 0;
            } else {
                // no log, sleep
                cli.scheduling = now + 50;
            }
        }

        s_view = CLI_VIEW_LOG; // mark
        break;
    }

    case STATE_CLI_FIND:
    {
        if ( cmd != -1 )
        {
            if (cmd=='l' || cmd=='L') { // atalho p/ log
                s_view = CLI_VIEW_LOG;
                s_log_header_done = false;
                cli.machine.state = STATE_CLI_LOG_MODE;
                break;
            }
            for(i = 0; i < cli.map_count; i++)
            {
                if(cmd == cli.map[i].menu_key)
                {
                    cli.map_idx = i;
                    cli.machine.state = STATE_CLI_CONFIG_MODE;
                    break;
                }
            }
        }
        break;
    }

    case STATE_CLI_HANDLER:
    {

        if ( now >= cli.scheduling || cmd != -1)
        {
            status = cli.map[cli.map_idx].handler(now,&(cli.map[cli.map_idx]), cmd);
            if(status == STATUS_DONE)
            {
                if(cli.map[cli.map_idx].tx->size <= 0)
                {
                    s_log_header_done = false;
                    cli.machine.state = STATE_CLI_LOG_MODE;
                }
            }
            else if(status == STATUS_ERROR)
            {
                s_log_header_done = false;
                cli.machine.state = STATE_CLI_LOG_MODE;
            }

            cli.scheduling = now + 1;
        }
        break;
    }

    case STATE_CLI_ERROR:
    {
        cli.scheduling = now + 1000;
        cli.machine.state = STATE_CLI_INIT;

        break;
    }


    }
    return status;
}


//LOG API
void cli_set_view(cli_view_mode_t v)        { s_view = v; }
void cli_set_log_level(cli_log_level_t lvl) { s_level = lvl; }

static uint16_t _log_count(void)
{
    return (uint16_t)(s_log_head - s_log_tail);
}

void cli_set_panel_log_lines(uint8_t n)
{
    if(n == 0) s_clear_log_lines = s_panel_log_lines;
    s_panel_log_lines = n > 8 ? 8 : n;
}

int cli_render_tail_log(char* out, int max)
{
    uint16_t i;
    int n = 0;

    if ( s_clear_log_lines )
    {
        n += update_display_clear_line(out);

        // N LOG lines
        for (i=0; i<s_clear_log_lines; ++i) {
            n += update_display_move_down(out+n,1);
            n += update_display_clear_line(out+n);
        }
        s_clear_log_lines = 0;
        return n;
    }
    if (!s_panel_log_lines) return 0;

       cli_log_entry_t tmp[8];
       uint16_t cnt = _log_copy_last(s_panel_log_lines, tmp); // do not consume from ring
       if (cnt > s_panel_log_lines) cnt = s_panel_log_lines;

       n=0;
       // title
       n += update_display_clear_line(out+n);
       n += snprintf(out+n, max-n, "---- LOG (last %u) ----\r\n", cnt);

       // N LOG lines
       for (i=0; i<s_panel_log_lines && n<max-32; ++i) {
           n += update_display_clear_line(out+n);
           if (i < cnt) {
               const cli_log_entry_t *e = &tmp[i];
               n += snprintf(out+n, max-n, "%8lu L%u %s\r\n",
                             (unsigned long)e->t_ms, (unsigned)e->level, e->msg);
           } else {
               n += snprintf(out+n, max-n, "\r\n");
           }
       }
       return n;
}

static uint16_t _log_copy_last(uint16_t max, cli_log_entry_t* dst)
{
    uint16_t i;
    if (max > 8) max = 8; //todo: verify this limit
    uint16_t avail = (uint16_t)(s_log_head - s_log_tail);
    if (avail == 0) return 0;
    if (avail > max) avail = max;
    uint16_t start = (uint16_t)(s_log_head - avail);
    for (i=0; i<avail; ++i)
        dst[i] = s_log_ring[(start + i) & (CLI_LOG_RING_SIZE - 1)];
    return avail;
}

static void _log_push(uint32_t t_ms, uint8_t level, const char *msg)
{
    uint16_t next = (uint16_t)((s_log_head + 1u) % CLI_LOG_RING_SIZE);
    if (next == s_log_tail) { // full -> write over older
        s_log_tail = (uint16_t)((s_log_tail + 1u) % CLI_LOG_RING_SIZE);
    }
    s_log_ring[s_log_head].t_ms  = t_ms;
    s_log_ring[s_log_head].level = level;
    // copy msg
    size_t i=0; for(; i<CLI_LOG_MSG_LEN-1 && msg[i]; ++i) s_log_ring[s_log_head].msg[i]=msg[i];
    s_log_ring[s_log_head].msg[i] = '\0';
    s_log_head = next;
}

static bool _log_pop(cli_log_entry_t *out)
{
    if (s_log_tail == s_log_head) return false;
    *out = s_log_ring[s_log_tail];
    s_log_tail = (uint16_t)((s_log_tail + 1u) % CLI_LOG_RING_SIZE);
    return true;
}


void cli_logf(cli_log_level_t lvl, const char *fmt, ...)
{
    if (lvl == LOG_OFF || lvl > s_level) return;
    char line[CLI_LOG_MSG_LEN];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (n < 0) return;

    DINT;                  // critico curtíssimo
    _log_push((uint32_t)my_time(NULL), (uint8_t)lvl, line);
    EINT;
}


// print menus
static int _print_quick_menu(char* out, int max)
{
    uint16_t i;
    int n = 0;
    n += update_display_for_menu(out);
    n += snprintf(out+n, max-n, "\r\n\x1b[7m\x1b[32m MENUS \x1b[27m\x1b[37m  \r\n");
    for (i=0; i<cli.map_count && n<max-32; ++i) {
        const char *name = cli.map[i].title ? cli.map[i].title : "Menu";
        n += snprintf(out+n, max-n, "  [%c] %s\r\n", (char)cli.map[i].menu_key, name);
    }
    n += snprintf(out+n, max-n, "  [l] Live Log\r\n");
    return n;
}

// return menu index or -1
static int _find_menu_by_key(int16_t key)
{
    uint16_t i;
    for (i=0; i<cli.map_count; ++i)
        if (cli.map[i].menu_key == key) return (int)i;
    return -1;
}

void print_line(char *buf, int *pn, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    *pn += vsnprintf(&(buf[*pn]), CLI_TEXT_BOX_SIZE - *pn, fmt, ap);
    va_end(ap);
    *pn += snprintf(&(buf[*pn]), CLI_TEXT_BOX_SIZE - *pn, "\r\n");
}

