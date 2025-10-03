//   HOW TO TEST
//acs37800_nb_t sensor;
//acs37800_nb_init(&sensor, &i2c_ctx, 0x40, 0.00305f, 0.0061f, 0.1875f, 0.0039f, 0.0625f, 0.01f);
//
//if (acs37800_nb_start_read_current_rms(&sensor) == 0) {
//    while (sensor.state == ACS37800_NB_BUSY) {
//        my_i2c_process(&i2c_ctx);
//        acs37800_nb_process(&sensor);
//    }
//    float current;
//    if (acs37800_nb_result(&sensor, &current) == 0) {
//        // uso do valor
//    }
//}



// acs37800_nb.c — Implementação do driver não bloqueante ACS37800 (corrigido)
#include "acs37800.h"
#include <string.h>

void acs37800_nb_init(acs37800_nb_t *dev,
                      i2c_context_t *ctx,
                      uint8_t addr,
                      float lsb_current,
                      float lsb_voltage,
                      float lsb_power,
                      float lsb_energy,
                      float lsb_temp,
                      float lsb_freq)
{
    memset(dev, 0, sizeof(*dev));
    dev->i2c_ctx      = ctx;
    dev->i2c_address  = addr;
    dev->lsb_current  = lsb_current;
    dev->lsb_voltage  = lsb_voltage;
    dev->lsb_power    = lsb_power;
    dev->lsb_energy   = lsb_energy;
    dev->state        = ACS37800_NB_IDLE;
    dev->op_type      = ACS_OP_NONE;
}

int acs37800_nb_start_read_reg32(acs37800_nb_t *dev, uint8_t reg) {
    if (!dev || dev->state != ACS37800_NB_IDLE) return -1;
    dev->reg_ptr = reg;
    dev->i2c_ctx->device_address = dev->i2c_address;
    dev->i2c_ctx->write_data = &dev->reg_ptr;
    dev->i2c_ctx->write_length = 1;
    dev->i2c_ctx->read_data = dev->raw;
    dev->i2c_ctx->read_length = 4;
    dev->i2c_ctx->request_start = true;
    dev->state = ACS37800_NB_BUSY;
    return 0;
}

int acs37800_nb_start_write_reg32(acs37800_nb_t *dev, uint8_t reg, uint16_t val) {
    if (!dev || dev->state != ACS37800_NB_IDLE) return -1;
    dev->write_data[0] = val >> 8;
    dev->write_data[1] = val & 0xFF;
    dev->reg_ptr = reg;
    dev->op_type = ACS_OP_WRITE_U32;
    dev->i2c_ctx->device_address = dev->i2c_address;
    dev->i2c_ctx->write_data = dev->write_data;
    dev->i2c_ctx->write_length = 4;
    dev->i2c_ctx->read_length = 0;
    dev->i2c_ctx->request_start = true;
    dev->state = ACS37800_NB_BUSY;
    return 0;
}


void acs37800_nb_process(acs37800_nb_t *dev) {
    i2c_context_t *ctx = dev->i2c_ctx;
    if (dev->state != ACS37800_NB_BUSY) return;

    if (ctx->state == I2C_STATE_ERROR) {
        dev->error = ctx->result;
        dev->state = ACS37800_NB_ERROR;
        return;
    }

    if (ctx->state != I2C_STATE_DONE)
        return;

    switch (dev->op_type) {
    case ACS_OP_READ_U32:
        break;
    case ACS_OP_READ_RMS:
        dev->raw_32 = dev->raw[3];
        dev->raw_32 = dev->raw_32<<8;
        dev->raw_32 += dev->raw[2];
        dev->raw_32 = dev->raw_32<<8;
        dev->raw_32 += dev->raw[1];
        dev->raw_32 = dev->raw_32<<8;
        dev->raw_32 += dev->raw[0];


        dev->voltage = (dev->raw[0] << 8) | dev->raw[1];
        dev->current = (dev->raw[2] << 8) | dev->raw[3];
        break;
    default:
        dev->error = -99;
        dev->state = ACS37800_NB_ERROR;
        return;
    }

    dev->state = ACS37800_NB_DONE;
}

int acs37800_nb_result(acs37800_nb_t *dev, float *out) {
    if (!dev) return -1;
    if (dev->state == ACS37800_NB_DONE) {
        if (out) *out = dev->result;
        dev->state = ACS37800_NB_IDLE;
        return 0;
    }
    if (dev->state == ACS37800_NB_ERROR) {
        int e = dev->error;
        dev->state = ACS37800_NB_IDLE;
        return e;
    }
    return -1;
}

int acs37800_nb_start_read_rms(acs37800_nb_t *dev) {

    dev->op_type = ACS_OP_READ_RMS;
    int res = acs37800_nb_start_read_reg32(dev, 0x20);
    return res;
}


int acs37800_nb_start_read_power(acs37800_nb_t *dev) {
    dev->op_type = ACS_OP_READ_POWER;
    return acs37800_nb_start_read_reg32(dev, 0x20);
}





//// acs37800.c
//#include <string.h>
//#include "acs37800.h"
//#include "my_i2c.h"
//
//int acs37800_nb_init(acs37800_handle_t *h,
//                     i2c_context_t    *i2c_ctx,
//                     uint8_t           address,
//                     float             lsb_amp)
//{
//    if (!h || !i2c_ctx) return -1;
//    memset(h, 0, sizeof(*h));
//    h->i2c_ctx     = i2c_ctx;
//    h->i2c_address = address;
//    h->lsb_amp     = lsb_amp;
//    h->state       = ACS37800_NB_IDLE;
//    h->phase       = ACS37800_PHASE_IDLE;
//    return 0;
//}
//
//int acs37800_read_current_nb_start(acs37800_handle_t *h)
//{
//    if (!h) return -1;
//    // auto‐reset interno
//    if (h->state != ACS37800_NB_IDLE) {
//        h->phase = ACS37800_PHASE_IDLE;
//        h->state = ACS37800_NB_IDLE;
//        h->error = 0;
//    }
//    // Fase 1: escrever ponteiro do registrador 0x00
//    static const uint8_t ptr = 0x00;
//    i2c_context_t *ctx = h->i2c_ctx;
//    ctx->device_address = h->i2c_address;
//    ctx->write_data     = (uint8_t*)&ptr;
//    ctx->write_length   = 1;
//    ctx->read_length    = 0;
//    my_i2c_start_write(ctx);
//
//    h->phase = ACS37800_PHASE_WAIT_PTR;
//    h->state = ACS37800_NB_BUSY;
//    return 0;
//}
//
//void acs37800_nb_process(acs37800_handle_t *h)
//{
//    if (!h || h->state != ACS37800_NB_BUSY) return;
//    i2c_context_t *ctx = h->i2c_ctx;
//
//    switch (h->phase) {
//    case ACS37800_PHASE_WAIT_PTR:
//        if (ctx->state == I2C_STATE_DONE) {
//            // Fase 2: iniciar leitura de 2 bytes
//            ctx->write_length = 0;
//            ctx->read_data    = h->raw;
//            ctx->read_length  = 2;
//            my_i2c_start_read(ctx);
//            h->phase = ACS37800_PHASE_WAIT_READ;
//        } else if (ctx->state == I2C_STATE_ERROR) {
//            h->error = ctx->result;
//            h->state = ACS37800_NB_ERROR;
//        }
//        break;
//
//    case ACS37800_PHASE_WAIT_READ:
//        if (ctx->state == I2C_STATE_DONE) {
//            // Monta o valor raw e converte para Ampères
//            h->raw_counts = (int16_t)((h->raw[0] << 8) | h->raw[1]);
//            h->current    = h->raw_counts * h->lsb_amp;
//            h->state      = ACS37800_NB_DONE;
//        } else if (ctx->state == I2C_STATE_ERROR) {
//            h->error = ctx->result;
//            h->state = ACS37800_NB_ERROR;
//        }
//        break;
//
//    default:
//        break;
//    }
//}
//
//int acs37800_nb_result(acs37800_handle_t *h, float *out_current)
//{
//    if (!h) return -1;
//    if (h->state == ACS37800_NB_DONE) {
//        if (out_current) *out_current = h->current;
//        h->state = ACS37800_NB_IDLE;
//        h->phase = ACS37800_PHASE_IDLE;
//        return 0;
//    }
//    if (h->state == ACS37800_NB_ERROR) {
//        int16_t e = h->error;
//        h->state = ACS37800_NB_IDLE;
//        h->phase = ACS37800_PHASE_IDLE;
//        return e;
//    }
//    return -1; // BUSY
//}
//
//int acs37800_nb_selftest(i2c_context_t *i2c_ctx,
//                         uint8_t         address,
//                         float           lsb_amp,
//                         float          *out_current)
//{
//    acs37800_handle_t h;
//    int ret;
//    float curr = 0.0f;
//
//    // 1) init
//    ret = acs37800_nb_init(&h, i2c_ctx, address, lsb_amp);
//    if (ret) return -1;
//
//    // 2) start
//    ret = acs37800_read_current_nb_start(&h);
//    if (ret) return -2;
//
//    // 3) polling até DONE/ERROR
//    while (h.state == ACS37800_NB_BUSY) {
//        my_i2c_process(i2c_ctx);
//        acs37800_nb_process(&h);
//    }
//
//    // 4) result
//    if (h.state == ACS37800_NB_DONE) {
//        ret = acs37800_nb_result(&h, &curr);
//        if (ret == 0 && out_current) *out_current = curr;
//        return ret;
//    }
//    return -3;
//}
