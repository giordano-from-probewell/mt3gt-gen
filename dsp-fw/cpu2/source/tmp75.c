// tmp75.c
#include <string.h>
#include "tmp75.h"
#include "my_i2c.h"
#include <F28x_Project.h>


int tmp75_nb_init(tmp75_nb_handle_t *h, tmp75_info_t *info, i2c_context_t *i2c_ctx)
{
    if (!h || !info || !i2c_ctx) return -1;
    memset(h, 0, sizeof(*h));
    h->info         = info;
    h->i2c_ctx      = i2c_ctx;
    h->state       = TMP75_NB_IDLE;
    h->info->error_counter = 0;
    return 0;
}

int tmp75_read_temp_nb_start(tmp75_nb_handle_t *h)
{
    if (!h || h->state == TMP75_NB_BUSY) return -1;

    h->info->cfg = 0;
    h->info->temperature       = -1000.0;
    h->i2c_ctx->i2c_regs       = h->info->i2c_regs;
    h->i2c_ctx->device_address = h->info->i2c_address;
    h->i2c_ctx->write_data     = &(h->info->cfg);
    h->i2c_ctx->write_length   = 1;
    h->i2c_ctx->read_data      = h->info->raw;
    h->i2c_ctx->read_length    = 2;
    my_i2c_start_read(h->i2c_ctx);
    h->state = TMP75_NB_BUSY;
    return 0;
}

void tmp75_nb_process(tmp75_nb_handle_t *h)
{
    if (!h || h->state != TMP75_NB_BUSY) return;

    if (h->i2c_ctx->state == I2C_STATE_DONE) {
        int16_t raw16 = ((int16_t)h->i2c_ctx->read_data[0] << 8) | h->i2c_ctx->read_data[1];
        raw16 >>= 4;
        h->info->temperature = raw16 * 0.0625f;
        h->state       = TMP75_NB_DONE;
    }
    else if (h->i2c_ctx->state == I2C_STATE_ERROR) {
        //h->info-> = ctx->result;
        h->state = TMP75_NB_ERROR;
    }

}

int tmp75_nb_result(tmp75_nb_handle_t *h, float32_t *out_temp)
{
    if (!h) return -1;
    if (h->state == TMP75_NB_DONE) {
        if (out_temp) *out_temp = h->info->temperature;
        h->state = TMP75_NB_IDLE;
        return 1;
    }
    if (h->state == TMP75_NB_ERROR) {
        int16_t e = h->i2c_ctx->result;
        return e;
    }
    return 0;
}

int tmp75_nb_selftest(tmp75_info_t *info, i2c_context_t *i2c_ctx)
{
    tmp75_nb_handle_t h;
    int16_t           err;
    float32_t         temp = 0.0;

    enum {
        STATE_INIT,
        STATE_READ,
        STATE_VERIFY,
        STATE_DONE
    } state = STATE_INIT;
    int16_t res = -1;



    // Test loop
    while (state != STATE_DONE) {
        my_i2c_process(i2c_ctx);
        tmp75_nb_process(&h);


        switch (state) {

        case STATE_INIT:
            err = tmp75_nb_init(&h, info, i2c_ctx);
            if (err) return -1;
            tmp75_read_temp_nb_start(&h);
            state = STATE_READ;
            break;
        case STATE_READ:
            res = tmp75_nb_result(&h, &temp);
            if (res == 1 ) {
                __asm("    ESTOP0");
                    state = STATE_VERIFY;
            } else if (res < 0) {
                __asm("    ESTOP0");
                return -2;
            }
            break;

        case STATE_VERIFY:
            __asm("    ESTOP0");
            return 0;

        default:
            break;
        }

    }
}



int tmp75_selftest(tmp75_info_t *info, i2c_context_t *i2c_ctx)
{


    int16_t res = -1;
    int ret = 0 ;



    //    __asm("    ESTOP0");

    //    // configure sensor: clear shutdown, continuous 12-bit
    //    uint8_t reg_ptr = 0x01, config_val = 0x60;
    //    ret = my_i2c_write(i2c_ctx->i2c_regs, info->i2c_address, &reg_ptr, 1, &config_val, 1);
    //    if (ret != 0) res -2;
    //
    //    DELAY_US(500000);
    uint8_t buf[2] ;
    //    ret = my_i2c_read(i2c_ctx->i2c_regs, info->i2c_address, 0, 1, (uint8_t*)buf, 2);
    //    DELAY_US(500);
    //    ret = my_i2c_read(i2c_ctx->i2c_regs, info->i2c_address, 0, 1, (uint8_t*)buf, 2);
    //    DELAY_US(500);
    //    ret = my_i2c_read(i2c_ctx->i2c_regs, info->i2c_address, 0, 1, (uint8_t*)buf, 2);
    //    DELAY_US(500);
    ret = my_i2c_read(i2c_ctx->i2c_regs, info->i2c_address, 0, 1, (uint8_t*)buf, 2);
    DELAY_US(500);

}
