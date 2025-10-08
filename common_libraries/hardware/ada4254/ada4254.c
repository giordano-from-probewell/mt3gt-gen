#include <stdint.h>
#include "hw_types.h"

#include "F2837xD_device.h"
#include "driverlib.h"
#include "device.h"

#include "ADA4254.h"
#include "CRC8.h"

#include "generic_definitions.h"


const float32_t ada4254_gain_in_floats[12] = {0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0};
const float32_t ada4254_gain_out_floats[3] = {1.0, 1.25, 1.375};

static ada4254_status_t ada4254_update_fgain(ada4254_t * this_channel);


/**
 * @brief Initializes the SPI module.
 *
 * @param base Base address of the SPI module.
 */
void SPI_init(uint16_t base)
{
    SPI_disableModule(base);
    SPI_setConfig(base, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA1, SPI_MODE_CONTROLLER, 1000000, 8);
    SPI_disableFIFO(base);
    SPI_setEmulationMode(base, SPI_EMULATION_STOP_AFTER_TRANSMIT);
    SPI_enableModule(base);
}



static inline int time_expired(my_time_t deadline, my_time_t now)
{
    //  overflow
    return (int32_t)(now - deadline) >= 0;
}



void ada4254_init_start(ada4254_init_ctx_t *ctx,
                        ada4254_t *channel,
                        uint16_t ch, uint16_t spi_base, uint16_t pin_cs)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->ch       = channel;
    ctx->ch_id    = ch;
    ctx->spi_base = spi_base;
    ctx->pin_cs   = pin_cs;

    channel->status   = ADA4254_STATUS_NOT_INITIALIZED;
    channel->channel  = ch;
    channel->spi_base = spi_base;
    channel->pin_cs   = pin_cs;

    ctx->tries = 0;
    ctx->st    = INIT_ST_READ_DIE;
    ctx->deadline_us = 0;
    ctx->last_status = ADA4254_STATUS_NOT_INITIALIZED;
}

ada4254_status_t ada4254_init_poll(ada4254_init_ctx_t *ctx, my_time_t now)
{
    ada4254_t *ch = ctx->ch;

    switch (ctx->st)
    {
    case INIT_ST_IDLE:
        return ch->status;

    case INIT_ST_READ_DIE:
        ada4254_read(ch, ADA4254_DIE_REV_ID, 1);
        ctx->st = INIT_ST_DECIDE_RESET;
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_DECIDE_RESET:
        if (ch->status == ADA4254_STATUS_OK) {
            ctx->st = INIT_ST_DONE;
            break;
        }
        if (ch->status == ADA4254_STATUS_PERMANENT_ERROR) {
            ctx->st = INIT_ST_ERROR;
            ch->status = ADA4254_STATUS_PERMANENT_ERROR;
            break;
        }
        if (ch->status != ADA4254_STATUS_WAIT_CAL_ERROR) {
            ctx->st = INIT_ST_RESET;
        } else {
            ctx->st = INIT_ST_ENABLE_CRC;
        }
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_RESET:
        ada4254_reset(ch);
        ctx->deadline_us = now + T_WAIT_AFTER_RESET_MS;
        ctx->st = INIT_ST_WAIT_AFTER_RESET;
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_WAIT_AFTER_RESET:
        if (!time_expired(ctx->deadline_us, now)) {
            return ADA4254_STATUS_NOT_INITIALIZED;
        }
        ctx->st = INIT_ST_ENABLE_CRC;
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_ENABLE_CRC:
        ada4254_crc_config(ch, true);
        ctx->st = INIT_ST_TEST_CONN;
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_TEST_CONN:
        if (ada4254_test_conn(ch) == ADA4254_STATUS_OK) {
            ctx->st = INIT_ST_VERIFY;
        } else {
            if (++ctx->tries > 10) {
                ch->status = ADA4254_STATUS_READ_ERROR;
                ctx->st = INIT_ST_ERROR;
            } else {
                ctx->st = INIT_ST_RESET;
            }
        }
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_VERIFY: {
        ada4254_status_t st = ada4254_verify(ch);
        if (st == ADA4254_STATUS_OK) {
            ctx->st = INIT_ST_DONE;
        } else if (st == ADA4254_STATUS_PERMANENT_ERROR) {
            ch->status = ADA4254_STATUS_PERMANENT_ERROR;
            ctx->st = INIT_ST_ERROR;
        } else {
            if (++ctx->tries > 10) {
                ch->status = ADA4254_STATUS_READ_ERROR;
                ctx->st = INIT_ST_ERROR;
            } else {
                ctx->deadline_us = now + T_WAIT_AFTER_VERIFY_MS;
                ctx->st = INIT_ST_WAIT_AFTER_VERIFY;
            }
        }
        return ADA4254_STATUS_NOT_INITIALIZED;
    }

    case INIT_ST_WAIT_AFTER_VERIFY:
        if (!time_expired(ctx->deadline_us, now)) {
            return ADA4254_STATUS_NOT_INITIALIZED;
        }
        ctx->st = INIT_ST_VERIFY;
        return ADA4254_STATUS_NOT_INITIALIZED;

    case INIT_ST_DONE:
        ch->status = ADA4254_STATUS_OK;
        ctx->st = INIT_ST_IDLE;
        return ADA4254_STATUS_OK;

    case INIT_ST_ERROR:
        ctx->st = INIT_ST_IDLE;
        return ch->status;
    }

    return ADA4254_STATUS_NOT_INITIALIZED;
}



void ada4254_set_gain_start(ada4254_gain_ctx_t* gctx,
                            ada4254_t* ch,
                            ada4254_in_gains_t in_gain,
                            ada4254_out_gains_t out_gain)
{
    memset(gctx, 0, sizeof(*gctx));
    gctx->ch       = ch;
    gctx->in_gain  = in_gain;
    gctx->out_gain = out_gain;
    gctx->st       = ADA4254_GAIN_ST_SEND_CMD;
    gctx->tries    = 0;
}

ada4254_status_t ada4254_set_gain_poll(ada4254_gain_ctx_t* gctx, my_time_t now)
{
    (void)now;
    ada4254_t* ch = gctx->ch;

    switch (gctx->st)
    {
    case ADA4254_GAIN_ST_IDLE:
        return ch->status;

    case ADA4254_GAIN_ST_SEND_CMD: {
        ada4254_status_t st = ada4254_set_gain(ch, gctx->in_gain, gctx->out_gain);
        if (st == ADA4254_STATUS_OK) {
            gctx->st = ADA4254_GAIN_ST_DONE;
            return ADA4254_STATUS_NOT_INITIALIZED;
        }
        if (st == ADA4254_STATUS_PERMANENT_ERROR) {
            ch->status = st;
            gctx->st = ADA4254_GAIN_ST_ERROR;
            return st;
        }
        if (++gctx->tries > 5) {
            ch->status = ADA4254_STATUS_READ_ERROR;
            gctx->st = ADA4254_GAIN_ST_ERROR;
            return ch->status;
        }
        return ADA4254_STATUS_NOT_INITIALIZED;
    }

    case ADA4254_GAIN_ST_DONE:
        ch->status = ADA4254_STATUS_OK;
        gctx->st = ADA4254_GAIN_ST_IDLE;
        return ADA4254_STATUS_OK;

    case ADA4254_GAIN_ST_ERROR:
        gctx->st = ADA4254_GAIN_ST_IDLE;
        return ch->status;

    default:
        return ADA4254_STATUS_ERROR;
    }
}




/**
 * @brief Initializes the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param ch Channel to be initialized.
 * @param spi_base SPI base address.
 * @param pin_cs Chip select pin.
 * @return Status of the initialization.
 */
//ada4254_status_t ada4254_init(ada4254_t * this_channel, uint16_t ch, uint16_t spi_base, uint16_t pin_cs)
//{
//    static int tries = 0;
//    //ada4254_status_t status;
//    this_channel->status = ADA4254_STATUS_NOT_INITIALIZED;
//    this_channel->channel = ch;
//    this_channel->spi_base = spi_base;
//    this_channel->pin_cs = pin_cs;
//
//    tries=0;
//
//    ada4254_read(this_channel, ADA4254_DIE_REV_ID, 1);
//
//    while( this_channel->status != ADA4254_STATUS_OK){
//
//        if(this_channel->status != ADA4254_STATUS_WAIT_CAL_ERROR)
//        {
//            ada4254_reset(this_channel);
//            DELAY_US(20000);
//            ada4254_crc_config(this_channel, true);
//        }
//
//        if(ada4254_test_conn(this_channel) == ADA4254_STATUS_OK)
//        {
//            if(ada4254_verify(this_channel) != ADA4254_STATUS_OK)
//                DELAY_US(40000);
//            if (this_channel->status == ADA4254_STATUS_PERMANENT_ERROR)
//            {
//                this_channel->status = ADA4254_STATUS_PERMANENT_ERROR;
//                return ADA4254_STATUS_PERMANENT_ERROR;
//            }
//        }
//        if(tries++>10)
//        {
//            this_channel->status = ADA4254_STATUS_READ_ERROR;
//            return ADA4254_STATUS_READ_ERROR;
//        }
//
//    }
//    this_channel->status = ADA4254_STATUS_OK;
//    return ADA4254_STATUS_OK;
//}

/**
 * @brief Verifies the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the verification.
 */
ada4254_status_t ada4254_verify(ada4254_t * this_channel)
{
    ada4254_status_t status_analog,status_dig;

    if(ada4254_read(this_channel, ADA4254_DIGITAL_ERR,1) == ADA4254_STATUS_OK)
    {
        this_channel->digital_err.data = this_channel->in_buffer[1] & 0x7c;
        status_dig = ADA4254_STATUS_OK;

        if(this_channel->digital_err.data)
        {
            this_channel->out_buffer[0] = ADA4254_DIGITAL_ERR;                                     // New Input gain
            this_channel->out_buffer[1] = this_channel->digital_err.data;                                // Concatenated Register
            this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
            ada4254_write(this_channel, 3);

            if(this_channel->digital_err.bits.CAL_BUSY)
            {
                status_dig = ADA4254_STATUS_WAIT_CAL_ERROR;
            }
            else if(this_channel->digital_err.bits.ROM_CRC_ERR)
            {
                status_dig = ADA4254_STATUS_PERMANENT_ERROR;
            }
            else
            {
                status_dig = ADA4254_STATUS_DIGITAL_ERROR;
            }
        }
    }

    if(ada4254_read(this_channel, ADA4254_ANALOG_ERR,1) == ADA4254_STATUS_OK)
    {
        this_channel->analog_err.data = this_channel->in_buffer[1] & 0xDF;
        status_analog = ADA4254_STATUS_OK;

        if(this_channel->analog_err.data)
        {
            this_channel->out_buffer[0] = ADA4254_ANALOG_ERR;                                     // New Input gain
            this_channel->out_buffer[1] = this_channel->analog_err.data;                                // Concatenated Register
            this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
            ada4254_write(this_channel, 3);

            status_analog = ADA4254_STATUS_ANALOG_ERROR;
        }
    }

    if(status_dig != ADA4254_STATUS_OK)
    {
        this_channel->status = status_dig;
        return status_dig;
    }
    else
    {
        this_channel->status = status_analog;
        return status_analog;
    }
}

/**
 * @brief Writes data to the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param msg_size Number of bytes to write.
 * @return Status of the write operation.
 */
ada4254_status_t ada4254_write(ada4254_t * this_channel, uint8_t msg_size)
{
    GPIO_writePin(this_channel->pin_cs,0);
    SPI_pollingFIFOTransaction(this_channel->spi_base, 8U, this_channel->out_buffer, this_channel->in_buffer, msg_size, 0);
    GPIO_writePin(this_channel->pin_cs,1);

    return ADA4254_STATUS_OK;
}

/**
 * @brief Reads data from the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param addr Address to read from.
 * @param msg_size Number of bytes to read.
 * @return Status of the read operation.
 */
ada4254_status_t ada4254_read(ada4254_t * this_channel, uint8_t addr, uint8_t msg_size)
{
    ada4254_status_t ret;
    uint8_t crc = 0;
    uint8_t cmd =  0x80 + addr;

    this_channel->out_buffer[0] = cmd;                                  // Read register with R flag at 7th bit

    if(this_channel->crc_on){
        this_channel->out_buffer[1] = CRC_8(this_channel->out_buffer, 1);       // CRC Calculation
        //msg_size+1 for send crc
        msg_size = msg_size+1;
    }

    ada4254_clear_input_buffer(this_channel);

    GPIO_writePin(this_channel->pin_cs,0);
    //msg_size+1 for send a dummy
    SPI_pollingFIFOTransaction(this_channel->spi_base, 8U, this_channel->out_buffer, this_channel->in_buffer, msg_size+1, 0);
    GPIO_writePin(this_channel->pin_cs,1);

    if(this_channel->crc_on){
        this_channel->in_buffer[0] = cmd;

        //verify only the first CRC
        crc = CRC_8(this_channel->in_buffer, 2);
        if(crc != this_channel->in_buffer[2])
        {
            ret = ADA4254_STATUS_CRC_ERROR;
        }
        else
        {
            ret = ADA4254_STATUS_OK;
        }
    }
    else
    {
        ret = ADA4254_STATUS_OK;
    }

    return ret;
}

/**
 * @brief Tests the connection to the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the connection test.
 */
ada4254_status_t ada4254_test_conn(ada4254_t * this_channel)
{
    ada4254_status_t ret;
    if(this_channel->crc_on == false)
    {
        ada4254_read(this_channel, ADA4254_DIE_REV_ID, 1);
        if (this_channel->in_buffer[1] == 0x30)
        {
            ret = ADA4254_STATUS_OK;
        }
        else{
            ret = ADA4254_STATUS_CONN_ERROR;
        }
    }
    else
    {
        //https://crccalc.com/ (CRC8(2f 30) = a6)
        ret = ada4254_read(this_channel, ADA4254_DIE_REV_ID,2);

        if (ret == ADA4254_STATUS_OK && this_channel->in_buffer[1] == 0x30)
        {
            ret = ADA4254_STATUS_OK;
        }
        else{
            ret =ADA4254_STATUS_CONN_ERROR;
        }
    }
    return ret;
}

/**
 * @brief Resets the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 */
void ada4254_reset(ada4254_t * this_channel)
{
    this_channel->input_gain    = IN_GAIN_1;                                    // Reset conditions
    this_channel->output_gain   = OUT_GAIN_1;
    this_channel->input_mux     = IN_MUX_CH1;
    this_channel->ext_mux       = 0x00;
    this_channel->crc_on        = false;

    // RESET PACKAGE (SPI Payload) = [ Chip Select | Reset Reg Addr | Reset Data ]

    this_channel->out_buffer[0] = ADA4254_DIGITAL_ERR_DIS;
    this_channel->out_buffer[1] = 0x7F;
    this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
    ada4254_write(this_channel, 3);

    this_channel->out_buffer[0] = 0x01;                                         // RESET register
    this_channel->out_buffer[1] = 0x01;                                         // Reset Value
    //    this_channel->out_buffer[2] = CRC_8(this_channel->out_buffer, 2);       // CRC8 Calculation
    ada4254_write(this_channel, 2);
}

/**
 * @brief Increments the input gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_inc_in_gain(ada4254_t * this_channel)
{
    uint8_t temp_gain_reg;

    if(this_channel->input_gain < IN_GAIN_128)
        this_channel->input_gain++;

    temp_gain_reg = ( (this_channel->output_gain >> 1) << 7) +                // Get LSB from output_gain (G4) and sets to Register MSB
            (this_channel->input_gain << 3) +                                   //
            this_channel->ext_mux;                                              // GPIO0 and GPIO1 (Ext Mux)

    this_channel->out_buffer[0] = ADA4254_GAIN_MUX;                             // New Input gain
    this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
    this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
    ada4254_write(this_channel, 3);

    ada4254_update_fgain(this_channel);

    return ADA4254_STATUS_OK;
}

/**
 * @brief Decrements the input gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_dec_in_gain(ada4254_t * this_channel)
{
    uint8_t temp_gain_reg;

    if(this_channel->input_gain > IN_GAIN_0_0625)
        this_channel->input_gain--;

    temp_gain_reg = ( (this_channel->output_gain >> 1) << 7) +                  // Get LSB from output_gain (G4) and sets to Register MSB
            (this_channel->input_gain << 3) +                                   //
            this_channel->ext_mux;                                              // GPIO0 and GPIO1 (Ext Mux)

    this_channel->out_buffer[0] = ADA4254_GAIN_MUX;                             // New Input gain
    this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
    this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
    ada4254_write(this_channel, 3);

    ada4254_update_fgain(this_channel);

    return ADA4254_STATUS_OK;
}

/**
 * @brief Increments the output gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_inc_out_gain(ada4254_t * this_channel)
{
    ada4254_status_t status;
    uint8_t temp_gain_reg;

    if(this_channel->output_gain < OUT_GAIN_1_375)
    {
        this_channel->output_gain++;

        temp_gain_reg = ( (this_channel->output_gain >> 1) << 7) +                  // Get LSB from output_gain (G4) and sets to Register MSB
                (this_channel->input_gain << 3) +                                   //
                this_channel->ext_mux;                                              // GPIO0 and GPIO1 (Ext Mux)

        this_channel->out_buffer[0] = ADA4254_GAIN_MUX;                             // New Input gain
        this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
        this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
        status = ada4254_write(this_channel, 3);

        if (status == ADA4254_STATUS_OK)
        {
            temp_gain_reg = 0x00;

            ada4254_read(this_channel, ADA4254_TEST_MUX, 1);

            temp_gain_reg = ( (this_channel->output_gain & 0x01) << 7) +
                    (this_channel->in_buffer[1] & 0x7F);

            this_channel->out_buffer[0] = ADA4254_TEST_MUX;                             // New Input gain
            this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
            this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);

            status = ada4254_write(this_channel, 3);
            if (status != ADA4254_STATUS_OK)
            {
                status = ADA4254_STATUS_GAIN_ERROR;
            }
        }
        else
        {
            status = ADA4254_STATUS_GAIN_ERROR;
        }

        ada4254_update_fgain(this_channel);
    }
    return status;
}

/**
 * @brief Decrements the output gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_dec_out_gain(ada4254_t * this_channel)
{
    ada4254_status_t status;
    uint8_t temp_gain_reg;

    if(this_channel->output_gain > OUT_GAIN_1)
    {
        this_channel->output_gain--;

        temp_gain_reg = ( (this_channel->output_gain & 0x10) << 6) +                // Get LSB from output_gain (G4) and sets to Register MSB
                (this_channel->input_gain << 3) +                                   //
                this_channel->ext_mux;                                              // GPIO0 and GPIO1 (Ext Mux)

        this_channel->out_buffer[0] = ADA4254_GAIN_MUX;                             // New Input gain
        this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
        this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
        status = ada4254_write(this_channel, 3);

        if (status == ADA4254_STATUS_OK)
        {
            temp_gain_reg = 0x00;

            ada4254_read(this_channel, ADA4254_TEST_MUX, 1);

            temp_gain_reg = ( (this_channel->output_gain & 0x01) << 7) +
                    (this_channel->in_buffer[1] & 0x7F);

            this_channel->out_buffer[0] = ADA4254_TEST_MUX;                             // New Input gain
            this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
            this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);

            status = ada4254_write(this_channel, 3);
            if (status != ADA4254_STATUS_OK)
            {
                status = ADA4254_STATUS_GAIN_ERROR;
            }
        }
        else
        {
            status = ADA4254_STATUS_GAIN_ERROR;
        }
        ada4254_update_fgain(this_channel);
    }

    return status;
}

/**
 * @brief Sets the gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param in_gain Input gain.
 * @param out_gain Output gain.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_set_gain(ada4254_t * this_channel, ada4254_in_gains_t in_gain, ada4254_out_gains_t out_gain)
{
    uint8_t temp_gain_reg;
    ada4254_status_t status;

    this_channel->input_gain = in_gain;                                         // Refresh saved input gain
    this_channel->output_gain = out_gain;

    temp_gain_reg = ( (this_channel->output_gain >> 1) << 7) +                  // Get LSB from output_gain (G4, bit 0) and sets to Register MSB
            (this_channel->input_gain << 3) +                                   //
            this_channel->ext_mux;                                              // GPIO0 and GPIO1 (Ext Mux)

    this_channel->out_buffer[0] = ADA4254_GAIN_MUX;                             // New Input gain
    this_channel->out_buffer[1] = temp_gain_reg;                                // Concatenated Register
    this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);       // CRC
    status = ada4254_write(this_channel, 3);
    if (status == ADA4254_STATUS_OK)
    {
        ada4254_read(this_channel, ADA4254_TEST_MUX, 1);                        // Read values from TEST_MUX

        temp_gain_reg = ( (this_channel->output_gain & 0x01) << 7) +            // Force G5 according to out_gain bit 1
                (this_channel->in_buffer[1] & 0x7F);                            // Keep other bits

        this_channel->out_buffer[0] = ADA4254_TEST_MUX;                         // Address of TEST_MUX
        this_channel->out_buffer[1] = temp_gain_reg;                            // Concatenated Register
        this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);   // CRC

        status = ada4254_write(this_channel, 3);
        if (status != ADA4254_STATUS_OK)
        {
            status = ADA4254_STATUS_GAIN_ERROR;
        }
    }
    else
    {
        status = ADA4254_STATUS_GAIN_ERROR;
    }

    ada4254_update_fgain(this_channel);

    return status;
}

/**
 * @brief Gets the current gain value for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Current gain value.
 */
float32_t ada4254_get_gain(ada4254_t * this_channel)
{
    if(this_channel->input_gain == IN_GAIN_0_0625)
        return 0.0625;
    if(this_channel->input_gain == IN_GAIN_0_125)
        return 0.125;
    if(this_channel->input_gain == IN_GAIN_0_25)
        return 0.25;
    if(this_channel->input_gain == IN_GAIN_0_5)
        return 0.5;
    if(this_channel->input_gain == IN_GAIN_1)
        return 1.0;
    if(this_channel->input_gain == IN_GAIN_2)
        return 2.0;
    if(this_channel->input_gain == IN_GAIN_4)
        return 4.0;
    if(this_channel->input_gain == IN_GAIN_8)
        return 8.0;
    if(this_channel->input_gain == IN_GAIN_16)
        return 16.0;
    if(this_channel->input_gain == IN_GAIN_32)
        return 32.0;
    if(this_channel->input_gain == IN_GAIN_64)
        return 64.0;
    if(this_channel->input_gain == IN_GAIN_128)
        return 128.0;

    return 0.0;
}

/**
 * @brief Configures CRC for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param state CRC state (true to enable, false to disable).
 * @return Status of the operation.
 */
ada4254_status_t ada4254_crc_config(ada4254_t * this_channel, bool state)
{


    uint8_t val;

    val = this_channel->in_buffer[1];

    if (state)  val |=  (1u << 5);
    else        val &= ~(1u << 5);

    this_channel->out_buffer[0] = ADA4254_DIGITAL_ERR_DIS;
    this_channel->out_buffer[1] = val;
    if (this_channel->crc_on) {
        this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
        return ada4254_write(this_channel, 3);
    } else {
        return ada4254_write(this_channel, 2);
    }



    //    ada4254_status_t ret = ADA4254_STATUS_ERROR;
    //
    //    if(this_channel->crc_on)
    //    {
    //        this_channel->out_buffer[0] = ADA4254_DIGITAL_ERR_DIS;
    //        this_channel->out_buffer[1] = this_channel->in_buffer[0] & (~(1 << 5) + (~state << 5));
    //        this_channel->out_buffer[2] = CRC_8(&this_channel->out_buffer[0], 2);
    //
    //        ret = ada4254_write(this_channel, 3);
    //    }
    //    else
    //    {
    //        this_channel->out_buffer[0] = ADA4254_DIGITAL_ERR_DIS;
    //        this_channel->out_buffer[1] = this_channel->in_buffer[0] & (~(1 << 5) + (~state << 5));
    //
    //        ret = ada4254_write(this_channel, 2);
    //    }
    //    if(ret == ADA4254_STATUS_OK)
    //    {
    //        this_channel->crc_on = state;
    //    }
    //    return ret;
}

/**
 * @brief Clears the input buffer for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_clear_input_buffer(ada4254_t * this_channel)
{
    uint8_t i;
    for (i=0; i<ADA4254_BUFFER_SIZE; i++)
    {
        this_channel->in_buffer[i] = 0x00;
    }

    return ADA4254_STATUS_OK;
}

/**
 * @brief Updates the floating point gain values for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
static ada4254_status_t ada4254_update_fgain(ada4254_t * this_channel)
{
    float32_t actual_gain_in = ada4254_gain_in_floats[(uint8_t)this_channel->input_gain];
    float32_t actual_gain_out = ada4254_gain_out_floats[(uint8_t)this_channel->output_gain];

    this_channel->f_gain = actual_gain_in * actual_gain_out;
    if(this_channel->f_gain == 0.0f)
    {
        this_channel->inv_f_gain = 0.0f; // avoid /0
    }
    else
    {
        this_channel->inv_f_gain = 1.0f / this_channel->f_gain;
    }
    return ADA4254_STATUS_OK;
}

/**
 * @brief Reads the gain values for the ADA4254 channel.
 * 
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.   
 */
ada4254_status_t ada4254_read_gain(ada4254_t * this_channel)
{
    ada4254_status_t ret = ADA4254_STATUS_GAIN_ERROR;

    uint8_t temp = 0x00;

    ret = ada4254_read(this_channel, ADA4254_GAIN_MUX, 1);
    if(ret == ADA4254_STATUS_OK)
    {
        this_channel->input_gain = (ada4254_in_gains_t)((0x78 & this_channel->in_buffer[1]) >> 3);
        this_channel->input_mux = (ada4254_in_mux_t)(0x03 & this_channel->in_buffer[1]);
        temp = (this_channel->in_buffer[1] & 0x80) >> 7;

        ret = ada4254_read(this_channel, ADA4254_TEST_MUX, 1);
        if(ret == ADA4254_STATUS_OK)
        {
            this_channel->output_gain = (ada4254_out_gains_t)(( (this_channel->in_buffer[1] & 0x80) >> 7 ) + (temp << 1));
        }

        ada4254_update_fgain(this_channel);
    }

    return ret;
}
