#include <cli_measures.h>
#include "cli.h"
#include "cli_main_menu.h"
#include "probewell_ascii.h"
#include "types.h"
#include "F28x_Project.h"
#include "string.h"
#include "sci_io.h"
#include "sci.h"
#include "application.h"
#include "generic_definitions.h"

#include "cli_main_menu.h"


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


//cli sigleton
/*static*/ cli_t cli;

cli_transmission_t _cli_tx;

cli_menu_t _cli_map[] = {
                                { .initialized = NULL, .tx = &(_cli_tx) ,.handler = cli_main_menu_handler,        .menu_key = '?' ,},
                                { .initialized = NULL, .tx = &(_cli_tx) ,.handler = cli_measures_menu_handler,    .menu_key = 'm' ,},
};


void _draw_logo(void);
void _clear_text_box(void);




//
// drawLogo -
//
void _draw_logo(void)
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

int16_t update_display_return(char *txt, int pos)
{
    int n = count_lines(txt,pos);

    int i = snprintf(&txt[pos], CLI_TEXT_BOX_SIZE, "0x1b[332m0x1b[%dA", n);

    return i;
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

int16_t update_display_save_position(char *txt)
{
    int i = 0;
    do{
        txt[i] = escSavePointer[i];
    } while(++i < 3);
    return i;
}

int16_t update_display_move_up(char *txt, int16_t n)
{
    int i = snprintf(txt, CLI_TEXT_BOX_SIZE, "\x1b[%dA", n);
    return i;
}

int16_t update_display_restore_position(char *txt)
{
    int i = 0;
    do{
        txt[i] = escRestorePointer[i];
    } while(++i < 3);
    return i;
}


void cli_update_display_raw(char *txt)
{
    size_t length = strlen(txt);
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)txt, length);
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

    //
    // Print a Logo
    //
    aux[0] = '\r';
    aux[1] = '\n';
    SCI_writeCharArray(CLI_SERIALPORT, (uint16_t *)aux, 2);
    _draw_logo();

    cli.initialized = INITIALIZED;





    _cli_map[0] = (cli_menu_t){
        .initialized = NULL,          // ou 0, se for bool/flag
        .tx         = &_cli_tx,
        .handler    = cli_main_menu_handler,
        .menu_key   = '?',
    };

    _cli_map[1] = (cli_menu_t){
        .initialized = NULL,
        .tx         = &_cli_tx,
        .handler    = cli_measures_menu_handler,
        .menu_key   = 'm',
    };

    return STATUS_DONE;
}




void _continue_non_blocking_transmission(const my_time_t time_actual, cli_transmission_t  *tx)
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



generic_status_t cli_processing (const my_time_t time_actual)
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
        _continue_non_blocking_transmission(time_actual, cli.tx);
        return STATUS_PROCESSING;
    }


    cmd = -1;
    if(SCI_isDataAvailableNonFIFO(CLI_SERIALPORT))
    {
        cmd = SCI_readCharNonBlocking(CLI_SERIALPORT);
    }


    //SCI_writeCharBlockingNonFIFO(CLI_SERIALPORT, 'B');

    switch(cli.machine.state)
    {

    case STATE_CLI_INIT:
    {
        cli.map_idx = 0;
        cli.initialized = INITIALIZED;
        cli.scheduling = time_actual + 100;
        cli.machine.state = STATE_CLI_FIND;
        break;
    }

    case STATE_CLI_CONFIG_MODE:
    {
        //        int n = update_display_for_menu(cli.tx->buffer);
        //        n += snprintf(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)  - n ,
        //                      "\r\n *** CONFIG MODE *** \r\n");
        //        cli.scheduling = time_actual + 100;
        //        cli.tx->size = n;
        cli.machine.state = STATE_CLI_HANDLER;
        break;
    }

    case STATE_CLI_LOG_MODE:
    {
        //        int n = update_display_for_data(cli.tx->buffer);
        //        n += snprintf(&(cli.tx->buffer[n]), sizeof(cli.tx->buffer)  - n ,
        //                      "\r\n *** LOG MODE *** \r\n");
        //        cli.scheduling = time_actual + 100;
        //        cli.tx->size = n;
        cli.machine.state = STATE_CLI_FIND;
        break;
    }

    case STATE_CLI_FIND:
    {
        if ( cmd != -1 )
        {
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

        if ( time_actual >= cli.scheduling || cmd != -1)
        {
            status = cli.map[cli.map_idx].handler(time_actual,&(cli.map[cli.map_idx]), cmd);
            if(status == STATUS_DONE)
            {
                if(cli.map[cli.map_idx].tx->size <= 0)
                {
                    cli.machine.state = STATE_CLI_LOG_MODE;
                }
            }
            else if(status == STATUS_ERROR)
            {
                cli.machine.state = STATE_CLI_LOG_MODE;
            }

            cli.scheduling = time_actual + 1;
        }
        break;
    }

    case STATE_CLI_ERROR:
    {
        cli.scheduling = time_actual + 1000;
        cli.machine.state = STATE_CLI_INIT;

        break;
    }


    }
    return status;
}







