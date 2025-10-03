#include <F28x_Project.h>
#include "my_i2c.h"

#define I2C_TIMEOUT        10000
#define I2C_CLOCK_LOW_TIME  0x270A
#define I2C_CLOCK_HIGH_TIME 0x270A

static bool i2c_initialized = false;


int16 my_i2c_initiate(volatile struct I2C_REGS *i2c_regs)
{
    if (i2c_initialized) return I2C_ERROR_ALREADY_INIT;

    i2c_regs->I2CMDR.bit.IRS = 0;            // disable I2C module
    i2c_regs->I2COAR.bit.OAR = 0;            // clear own address
    i2c_regs->I2CPSC.bit.IPSC = 1;           // prescaler = 1
    i2c_regs->I2CIER.all = 0;                // disable interrupts
    i2c_regs->I2CCLKL = I2C_CLOCK_LOW_TIME;  // set SCL low time
    i2c_regs->I2CCLKH = I2C_CLOCK_HIGH_TIME; // set SCL high time
    i2c_regs->I2CCNT = 0;                    // clear byte counter
    i2c_regs->I2CMDR.all = 0;                // clear control register
    i2c_regs->I2CMDR.bit.FREE = 1;           // free-run mode
    i2c_regs->I2CMDR.bit.MST  = 1;           // master mode
    i2c_regs->I2CMDR.bit.RM   = 1;           // receiver mode (for read)
    i2c_regs->I2CMDR.bit.BC   = 0;           // broadcast disabled
    i2c_regs->I2CMDR.bit.IRS  = 1;           // enable I2C


    i2c_initialized = true;
    return I2C_ERROR_NONE;
}

int16 my_i2c_terminate(volatile struct I2C_REGS *i2c_regs)
{
    i2c_regs->I2CMDR.bit.IRS = 0; // disable I2C module
    i2c_initialized = false;
    return I2C_ERROR_NONE;
}


int16 my_i2c_read(volatile struct I2C_REGS *i2c_regs,
                  const uint8_t device_address,
                  const uint8_t *register_address,
                  const uint16_t register_address_count,
                  uint8_t *data, const uint16_t count)
{
    uint32_t timeout; uint16_t idx, addr_count;
    if (!i2c_initialized) return I2C_ERROR_NOT_INIT;
    timeout = 0;
    while (i2c_regs->I2CMDR.bit.STP == 1)
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    i2c_regs->I2CSAR.bit.SAR = device_address;
    i2c_regs->I2CMDR.bit.MST = 1;    // master mode
    i2c_regs->I2CMDR.bit.TRX = 1;    // write mode
    i2c_regs->I2CMDR.bit.STT = 1;    // start
    timeout = 0;
    while (i2c_regs->I2CMDR.bit.STT == 1)
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    if (i2c_regs->I2CSTR.bit.NACK) {
        i2c_regs->I2CSTR.bit.NACK = 1;
        i2c_regs->I2CMDR.bit.STP = 1; // stop on NACK
        return I2C_ERROR_BUS;
    }
    addr_count = 0;
    while (addr_count < register_address_count) {
        i2c_regs->I2CDXR.bit.DATA = register_address[addr_count++];
        timeout = 0;
        while (!i2c_regs->I2CSTR.bit.XRDY)
            if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    }
    i2c_regs->I2CMDR.bit.TRX = 0; // read mode
    i2c_regs->I2CMDR.bit.STT = 1; // repeated start
    timeout = 0;
    while (i2c_regs->I2CMDR.bit.STT == 1)
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    idx = 0;
    while (idx < count) {
        if (idx == (count - 1)) i2c_regs->I2CMDR.bit.STP = 1;
        timeout = 0;
        while (!i2c_regs->I2CSTR.bit.RRDY)
            if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
        data[idx++] = i2c_regs->I2CDRR.bit.DATA;
        i2c_regs->I2CSTR.bit.RRDY = 1;
    }
    timeout = 0;
    while ((i2c_regs->I2CMDR.bit.STP == 1) || (i2c_regs->I2CSTR.bit.BB == 1))
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    return I2C_ERROR_NONE;
}


int16 my_i2c_write(volatile struct I2C_REGS *i2c_regs,
                   const uint8_t device_address,
                   const uint8_t *register_address,
                   const uint16_t register_address_count,
                   uint8_t *data, const uint16_t count)
{
    uint32_t timeout; uint16_t idx, addr_count;
    if (!i2c_initialized) return I2C_ERROR_NOT_INIT;
    timeout = 0;
    while (i2c_regs->I2CMDR.bit.STP == 1)
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    i2c_regs->I2CSAR.bit.SAR = device_address;
    i2c_regs->I2CMDR.bit.MST = 1;    // master mode
    i2c_regs->I2CMDR.bit.TRX = 1;    // write mode
    i2c_regs->I2CMDR.bit.STT = 1;    // start
    timeout = 0;
    while (i2c_regs->I2CMDR.bit.STT == 1)
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    if (i2c_regs->I2CSTR.bit.NACK) {
        i2c_regs->I2CSTR.bit.NACK = 1; i2c_regs->I2CMDR.bit.STP = 1;
        return I2C_ERROR_BUS;
    }
    addr_count = 0;
    while (addr_count < register_address_count) {
        i2c_regs->I2CDXR.bit.DATA = register_address[addr_count++];
        timeout = 0;
        while (!i2c_regs->I2CSTR.bit.XRDY)
            if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    }
    idx = 0;
    while (idx < count) {
        i2c_regs->I2CDXR.bit.DATA = data[idx++];
        timeout = 0;
        while (!i2c_regs->I2CSTR.bit.XRDY)
            if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
        if (idx == count) i2c_regs->I2CMDR.bit.STP = 1;
    }
    timeout = 0;
    while ((i2c_regs->I2CMDR.bit.STP == 1) || (i2c_regs->I2CSTR.bit.BB == 1))
        if (timeout++ > I2C_TIMEOUT) return I2C_ERROR_BUSY;
    return I2C_ERROR_NONE;
}





// --- API não bloqueante ---


int16 my_i2c_restart(volatile struct I2C_REGS *i2c_regs)
{
    i2c_regs->I2CMDR.bit.IRS = 0;
    i2c_regs->I2COAR.bit.OAR = 0;
    i2c_regs->I2CPSC.bit.IPSC = 1;
    i2c_regs->I2CIER.all = 0;
    i2c_regs->I2CCLKL = I2C_CLOCK_LOW_TIME;
    i2c_regs->I2CCLKH = I2C_CLOCK_HIGH_TIME;
    i2c_regs->I2CCNT = 0;
    i2c_regs->I2CMDR.all = 0;
    i2c_regs->I2CMDR.bit.FREE = 1;
    i2c_regs->I2CMDR.bit.MST  = 1;
    i2c_regs->I2CMDR.bit.RM   = 1;
    i2c_regs->I2CMDR.bit.BC   = 0;
    i2c_regs->I2CMDR.bit.IRS  = 1;

    i2c_initialized = true;
    return I2C_ERROR_NONE;
}

void my_i2c_start_write(i2c_context_t *ctx)
{
    ctx->write_index = 0;
    ctx->read_index = 0;
    ctx->request_start = true;
    ctx->timeout     = 0;
}

void my_i2c_start_read(i2c_context_t *ctx)
{
    ctx->write_index = 0;
    ctx->read_index = 0;
    ctx->request_start = true;
    ctx->timeout    = 0;
}

void my_i2c_process(i2c_context_t *ctx)
{
    volatile struct I2C_REGS *regs;
    if (!ctx) return;
    regs = ctx->i2c_regs;

    if (  ctx->state == I2C_STATE_DONE || ctx->state == I2C_STATE_IDLE ){
        if(ctx->request_start) {
            ctx->state          = I2C_STATE_START;
            ctx->request_start  = false;
            ctx->result         = I2C_ERROR_NONE;
        }
        return;
    }


    if (ctx->state == I2C_STATE_ERROR)
    {
        my_i2c_restart(regs);
        ctx->timeout    = 0;
        ctx->state  = I2C_STATE_IDLE;
        ctx->result = I2C_ERROR_NONE;
        return;
    }

    // Timeout global
    if (++ctx->timeout > I2C_TIMEOUT) {
        ctx->state  = I2C_STATE_ERROR;
        ctx->result = I2C_ERROR_BUSY;
        return;
    }


    switch (ctx->state) {

    case I2C_STATE_START:
        if (regs->I2CMDR.bit.STP == 0) {

            regs->I2CSAR.bit.SAR = ctx->device_address;
            regs->I2CMDR.bit.MST = 1;
            regs->I2CMDR.bit.TRX = (ctx->write_length > 0);
            regs->I2CMDR.bit.STT = 1;
            ctx->state = I2C_STATE_WAIT_START;
            ctx->timeout = 0;
        }
        break;

    case I2C_STATE_WAIT_START:
        if (regs->I2CMDR.bit.STT == 0) {
            ctx->state = (ctx->write_length > 0) ? I2C_STATE_SEND_REG_ADDR : I2C_STATE_RESTART;
            ctx->timeout = 0;
        }
        if (regs->I2CSTR.bit.NACK) {
            regs->I2CSTR.bit.NACK = 1;
            regs->I2CMDR.bit.STP = 1;
            ctx->state  = I2C_STATE_DONE;
            ctx->result = I2C_ERROR_BUS;
        }
        break;

    case I2C_STATE_SEND_REG_ADDR:
        if (regs->I2CSTR.bit.XRDY) {
            if (ctx->write_index < ctx->write_length) {
                regs->I2CDXR.bit.DATA = ctx->write_data[ctx->write_index++];
                ctx->timeout = 0;
            } else {
                if (ctx->read_length > 0) {
                    ctx->state = I2C_STATE_RESTART;
                } else {
                    regs->I2CMDR.bit.STP = 1;
                    ctx->state = I2C_STATE_WAIT_STOP;
                }
                ctx->timeout = 0;
            }
        }
        break;

    case I2C_STATE_RESTART:
        regs->I2CMDR.bit.TRX = 0;
        regs->I2CSTR.bit.RRDY = 1;
        regs->I2CMDR.bit.STT = 1;

        ctx->state = I2C_STATE_WAIT_RESTART;
        ctx->timeout = 0;
        break;

    case I2C_STATE_WAIT_RESTART:
        if (regs->I2CMDR.bit.STT == 0) {
            ctx->state = I2C_STATE_READ;
            ctx->timeout = 0;
        }
        break;

    case I2C_STATE_READ:
        if (regs->I2CSTR.bit.RRDY) {
            ctx->read_data[ctx->read_index++] = regs->I2CDRR.bit.DATA;
            if (ctx->read_index == ctx->read_length-1) {
                regs->I2CMDR.bit.STP = 1;
                ctx->state = I2C_STATE_WAIT_STOP;
            }
            ctx->timeout = 0;
        }
        break;

    case I2C_STATE_WAIT_STOP:
        if ((regs->I2CMDR.bit.STP == 0) && (regs->I2CSTR.bit.BB == 0)) {
            ctx->read_data[ctx->read_index] = regs->I2CDRR.bit.DATA;
            ctx->state  = I2C_STATE_DONE;
            ctx->result = I2C_ERROR_NONE;
        }
        break;





    default:
        break;
    }
}
