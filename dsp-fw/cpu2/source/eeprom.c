// eeprom.c
#include <F28x_Project.h>
#include <errno.h>
#include <string.h>
#include "eeprom.h"

// --- API bloqueante (copie exatamente do seu eeprom.c que já funciona) ---

static int validate_eeprom_info(const eeprom_info_t *eeprom)
{
    if (!eeprom || !eeprom->i2c_regs || eeprom->memory_size == 0 || eeprom->page_size == 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

static int eeprom_handle_error(int i2c_error_code)
{
    switch (i2c_error_code) {
    case 0:  return 0;
    case -1: return ENXIO;
    case -2: return EBUSY;
    case -3: return EIO;
    case -4: return EINVAL;
    case -5: return EAGAIN;
    default: return -1;
    }
}

int eeprom_initiate(eeprom_info_t *eeprom)
{
    if (validate_eeprom_info(eeprom)) return -1;
    eeprom->address_pointer = 0;
    eeprom->error_counter   = 0;
    errno = 0;
    return 0;
}

int eeprom_terminate(eeprom_info_t *eeprom)
{
    if (validate_eeprom_info(eeprom)) return -1;
    errno = 0;
    return 0;
}

int eeprom_read(eeprom_info_t *eeprom, char *buf, unsigned count)
{
    if (validate_eeprom_info(eeprom) || !buf || count==0) {
        errno = EINVAL; return -1;
    }
    uint16_t end = eeprom->address_pointer + count;
    unsigned read_n = (end>eeprom->memory_size) ? (eeprom->memory_size - eeprom->address_pointer) : count;
    if (read_n==0) { errno=0; return 0; }
    uint8_t addr[2] = { eeprom->address_pointer>>8, eeprom->address_pointer&0xFF };
    int ret = my_i2c_read(eeprom->i2c_regs, eeprom->i2c_address, addr, 2, (uint8_t*)buf, read_n);
    if (ret) { errno = eeprom_handle_error(ret); return -1; }
    eeprom->address_pointer += read_n;
    errno = 0;
    return read_n;
}

int eeprom_write(eeprom_info_t *eeprom, const char *buf, unsigned count)
{
    if (validate_eeprom_info(eeprom) || !buf || count==0) {
        errno=EINVAL; return -1;
    }
    if (eeprom->address_pointer>=eeprom->memory_size) { errno=0; return 0; }
    uint16_t page_rem = eeprom->page_size - (eeprom->address_pointer % eeprom->page_size);
    unsigned write_n = (count>page_rem) ? page_rem : count;
    uint8_t addr[2] = { eeprom->address_pointer>>8, eeprom->address_pointer&0xFF };
    int ret = my_i2c_write(eeprom->i2c_regs, eeprom->i2c_address, addr, 2, (uint8_t*)buf, write_n);
    if (ret) { errno = eeprom_handle_error(ret); return -1; }
    eeprom->address_pointer += write_n;
    errno = 0;
    return write_n;
}

long eeprom_seek(eeprom_info_t *eeprom, long offset, int origin)
{
    if (validate_eeprom_info(eeprom)) return -1;
    long newpos;
    switch(origin){
    case EEPROM_SEEK_SET: newpos=offset; break;
    case EEPROM_SEEK_CUR: newpos=eeprom->address_pointer+offset; break;
    case EEPROM_SEEK_END: newpos=eeprom->memory_size+offset; break;
    default: errno=EINVAL; return -1;
    }
    if (newpos<0 || newpos>=eeprom->memory_size) { errno=EINVAL; return -1; }
    eeprom->address_pointer = (uint16_t)newpos;
    errno=0;
    return newpos;
}

int eeprom_ctrl(eeprom_info_t *eeprom, int cmd, void *argp)
{
    if (validate_eeprom_info(eeprom)) return -1;
    switch(cmd){
    case EEPROM_CTRL_ERROR_COUNTER_GET:
        if(!argp){ errno=EINVAL; return -1; }
        *(uint16_t*)argp = eeprom->error_counter;
        errno=0; return 0;
    case EEPROM_CTRL_ERROR_COUNTER_CLEAR:
        eeprom->error_counter=0; errno=0; return 0;
    case EEPROM_CTRL_MEMORY_INFO:
        if(!argp){ errno=EINVAL; return -1; }
        ((eeprom_info_t*)argp)->memory_size = eeprom->memory_size;
        ((eeprom_info_t*)argp)->page_size   = eeprom->page_size;
        errno=0; return 0;
    default:
        errno=ENOTTY; return -1;
    }
}

// Non-blocking implementations

int eeprom_nb_init(eeprom_nb_handle_t *h, eeprom_info_t *info, i2c_context_t *i2c_ctx)
{
    if (!h || !info || !i2c_ctx) return -1;
    h->info    = info;
    h->i2c_ctx = i2c_ctx;
    h->state   = EEPROM_NB_IDLE;
    h->result  = 0;
    h->info->error_counter = 0;
    return 0;
}

int eeprom_read_nb_start(eeprom_nb_handle_t *h, char *buf, unsigned count)
{
    if (!h || h->state == EEPROM_NB_BUSY || !buf || count == 0) return -1;
    uint16_t remain = h->info->memory_size - h->info->address_pointer;
    unsigned to_read = (count > remain) ? remain : count;
    if (!to_read) {
        h->state  = EEPROM_NB_DONE;
        h->result = 0;
        return 0;
    }
    uint16_t addr = h->info->address_pointer;

    h->write_buf[0] = (uint8_t)(addr >> 8);
    h->write_buf[1] = (uint8_t)(addr & 0xFF);

    // configure shared I2C context
    h->i2c_ctx->i2c_regs       = h->info->i2c_regs;
    h->i2c_ctx->device_address = h->info->i2c_address;
    h->i2c_ctx->write_data     = h->write_buf;
    h->i2c_ctx->write_length   = 2;
    h->i2c_ctx->read_data      = (uint8_t*)buf;
    h->i2c_ctx->read_length    = to_read;

    my_i2c_start_write(h->i2c_ctx);
    h->xfer_len = to_read;
    h->state    = EEPROM_NB_BUSY;
    return 0;
}

int eeprom_write_nb_start(eeprom_nb_handle_t *h, const char *buf, unsigned count)
{
    if (!h || h->state == EEPROM_NB_BUSY || !buf || count == 0) return -1;
    uint16_t page_rem = h->info->page_size - (h->info->address_pointer % h->info->page_size);
    unsigned to_write = (count > page_rem) ? page_rem : count;
    if (!to_write) {
        h->state  = EEPROM_NB_DONE;
        h->result = 0;
        return 0;
    }
    uint16_t addr = h->info->address_pointer;
    h->write_buf[0] = (uint8_t)(addr >> 8);
    h->write_buf[1] = (uint8_t)(addr & 0xFF);
    memcpy(&h->write_buf[2], buf, to_write);

    h->i2c_ctx->i2c_regs       = h->info->i2c_regs;
    h->i2c_ctx->device_address = h->info->i2c_address;
    h->i2c_ctx->write_data     = h->write_buf;
    h->i2c_ctx->write_length   = to_write + 2;
    h->i2c_ctx->read_length    = 0;

    my_i2c_start_write(h->i2c_ctx);
    h->xfer_len = to_write;
    h->state    = EEPROM_NB_BUSY;
    return 0;
}

void eeprom_nb_process(eeprom_nb_handle_t *h)
{
    if (!h || h->state != EEPROM_NB_BUSY) return;
    i2c_context_t *ctx = h->i2c_ctx;
    if (ctx->state == I2C_STATE_DONE) {
        h->info->address_pointer += h->xfer_len;
        h->result = h->xfer_len;
        h->state  = EEPROM_NB_DONE;
    }
    else if (ctx->state == I2C_STATE_ERROR) {
        h->info->error_counter++;
        h->result = ctx->result;
        h->state  = EEPROM_NB_ERROR;
    }
}

int eeprom_nb_result(eeprom_nb_handle_t *h)
{
    if (!h) return -1;
    if (h->state == EEPROM_NB_DONE || h->state == EEPROM_NB_ERROR)
        return h->result;
    return -1; // still busy
}





/**
 * @brief Exemplo de função estanque para testar a API não-bloqueante da EEPROM.
 *
 * Escreve 10 bytes de exemplo na EEPROM e lê de volta, verificando a correspondência.
 * Utiliza apenas as funções não-bloqueantes e processa o I²C em um loop interno.
 *
 * @param info    Ponteiro para configuração da EEPROM (eeprom_info_t).
 * @param i2c_ctx Ponteiro para contexto I2C compartilhado (i2c_context_t).
 * @return 0 em caso de sucesso (dados iguais), <0 em caso de erro:
 *         -1: falha ao escrever
 *         -2: falha ao ler
 *         -3: dados lidos não correspondem
 */
int eeprom_nb_selftest(eeprom_info_t *info, i2c_context_t *i2c_ctx)
{


    const char write_data[3] = {0xCC,0xBB,0xAA};
    char read_data[10] = {0};

    eeprom_nb_handle_t nb;
    enum {
        STATE_WRITE_INIT,
        STATE_WAIT_WRITE,
        STATE_READ_INIT1,
        STATE_WAIT_READ1,
        STATE_READ_INIT2,
        STATE_WAIT_READ2,
        STATE_VERIFY,
        STATE_DONE
    } state = STATE_READ_INIT1;
    int16_t res = -1;





    // Loop de teste
    while (state != STATE_DONE) {
        my_i2c_process(i2c_ctx);
        eeprom_nb_process(&nb);

        switch (state) {

        case STATE_READ_INIT1:
            __asm("    ESTOP0");


            // Posiciona cursor no início
            eeprom_seek(info, 0, EEPROM_SEEK_SET);
            // Re-inicializa handle
            eeprom_nb_init(&nb, info, i2c_ctx);
            // Inicia leitura não-bloqueante
            res = eeprom_read_nb_start(&nb, read_data, sizeof(read_data));
            if (res < 0) return -2;
            state = STATE_WAIT_READ1;
            break;

        case STATE_WAIT_READ1:
            if (nb.state == EEPROM_NB_DONE) {
                __asm("    ESTOP0");
                res = eeprom_nb_result(&nb);
                state = STATE_WRITE_INIT;
            } else if (nb.state == EEPROM_NB_ERROR) {
                __asm("    ESTOP0");
                return -2;
            }
            break;


        case STATE_WRITE_INIT:
            // Posiciona cursor no início
            __asm("    ESTOP0");
            eeprom_seek(info, 0, EEPROM_SEEK_SET);
            // Inicializa handle
            eeprom_nb_init(&nb, info, i2c_ctx);
            // Inicia escrita não-bloqueante
            res = eeprom_write_nb_start(&nb, write_data, sizeof(write_data));
            if (res < 0) return -1;
            state = STATE_WAIT_WRITE;
            break;

        case STATE_WAIT_WRITE:
            if (nb.state == EEPROM_NB_DONE) {
                __asm("    ESTOP0");
                res = eeprom_nb_result(&nb);  // bytes escritos
                state = STATE_READ_INIT2;
            } else if (nb.state == EEPROM_NB_ERROR) {
                __asm("    ESTOP0");
                return -1;
            }
            break;

        case STATE_READ_INIT2:
            __asm("    ESTOP0");
            // Posiciona cursor no início
            eeprom_seek(info, 0, EEPROM_SEEK_SET);
            // Re-inicializa handle
            eeprom_nb_init(&nb, info, i2c_ctx);
            // Inicia leitura não-bloqueante
            res = eeprom_read_nb_start(&nb, read_data, sizeof(read_data));
            if (res < 0) return -2;
            state = STATE_WAIT_READ2;
            break;

        case STATE_WAIT_READ2:
            if (nb.state == EEPROM_NB_DONE) {
                __asm("    ESTOP0");
                res = eeprom_nb_result(&nb);  // bytes lidos
                state = STATE_VERIFY;
            } else if (nb.state == EEPROM_NB_ERROR) {
                __asm("    ESTOP0");
                return -2;
            }
            break;

        case STATE_VERIFY:
            // Verifica correspondência
            __asm("    ESTOP0");
            if (res == (int16_t)sizeof(write_data) &&
                memcmp(write_data, read_data, res) == 0) {
                __asm("    ESTOP0");
                return 0;
            } else {
                __asm("    ESTOP0");
                return -3;
            }

        default:
            break;
        }
    }
    return -3;  // não deveria chegar aqui
}




//-----------------------------
// Self-test function for blocking EEPROM API
//-----------------------------
/**
 * @brief Exemplo de função estanque para testar a API bloqueante da EEPROM.
 *
 * Grava e lê um texto na EEPROM utilizando as funções bloqueantes.
 *
 * @return 0 em caso de sucesso, valor negativo em caso de falha:
 *         -1: falha na inicialização do I2C
 *         -2: falha na iniciação da EEPROM
 *         -3: falha na leitura inicial
 *         -4: falha na busca para escrita
 *         -5: falha na escrita
 *         -6: falha na busca para leitura
 *         -7: falha na leitura de verificação
 *         -8: mismatch de dados
 */


#define TEST_I2C_MODULE     &I2cbRegs
#define TEST_EEPROM_ADDRESS 0x50
#define TEST_MEMORY_SIZE    4096
#define TEST_PAGE_SIZE      100

int eeprom_sync_selftest() {
    // Dados de teste
    const char write_data[] = "AA";
    char read_data[sizeof(write_data)];
    int16_t ret;


    // Configura informações da EEPROM
    eeprom_info_t info = {
        .i2c_regs        = TEST_I2C_MODULE,
        .i2c_address     = 0x55,
        .memory_size     = TEST_MEMORY_SIZE,
        .page_size       = TEST_PAGE_SIZE,
        .address_pointer = 0,
        .error_counter   = 0
    };

    // Inicia EEPROM
    ret = eeprom_initiate(&info);
    if (ret != 0) return -2;
    __asm("    ESTOP0");
    // Leitura inicial (opcional)
    if (eeprom_seek(&info, 0, EEPROM_SEEK_SET) < 0) return -3;
    if (eeprom_read(&info, read_data, 3) != 3) return -3;
    __asm("    ESTOP0");
    // Escrita de teste
    if (eeprom_seek(&info, 0, EEPROM_SEEK_SET) < 0) return -4;
    if (eeprom_write(&info, write_data, sizeof(write_data)) != sizeof(write_data)) return -5;
    __asm("    ESTOP0");
    // Leitura de verificação
    if (eeprom_seek(&info, 0, EEPROM_SEEK_SET) < 0) return -6;
    if (eeprom_read(&info, read_data, sizeof(read_data)) != sizeof(read_data)) return -7;
    __asm("    ESTOP0");
    // Verifica dados
    if (strcmp(write_data, read_data) != 0) return -8;

    // Finaliza EEPROM
    ret = eeprom_terminate(&info);
    return ret;
}


