/**
 * @file ADA4254.h
 * @brief Header file for ADA4254 driver.
 */

#ifndef ADA4254_H_
#define ADA4254_H_

#include "device.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "my_time.h"
#include "CRC8.h"

#define ADA4254_BUFFER_SIZE             20u         // Internal buffer size of each channel
#define ADA_MAX_TRIES                   10u          // Max tries to communicate with ADA4254

#define ADA4254_GAIN_MUX                0x00        // GAIN_MUX REGISTER
#define ADA4254_RESET                   0x01        // SOFTWARE RESET REGISTER
#define ADA4254_SYNC_CFG                0x02        // CLOCK SYNCHRONIZATION CONFIGURATION REGISTER
#define ADA4254_DIGITAL_ERR             0x03        // DIGITAL ERROR REGISTER
#define ADA4254_ANALOG_ERR              0x04        // ANALOG ERROR REGISTER
#define ADA4254_GPIO_DATA               0x05        // GPIO DATA REGISTER
#define ADA4254_INPUT_MUX               0x06        // INTERNAL MUX CONTROL REGISTER
#define ADA4254_WB_DETECT               0x07        // WIRE BREAK DETECT
#define ADA4254_GPIO_DIR                0x08        // GPIO DIRECTION REGISTER
#define ADA4254_SCS                     0x09        // SEQUENTIAL CHIP SELECT REGISTER
#define ADA4254_ANALOG_ERR_DIS          0x0A        // ANALOG ERROR MASK REGISTER
#define ADA4254_DIGITAL_ERR_DIS         0x0B        // DIGITAL ERROR MASK REGISTER
#define ADA4254_SF_CFG                  0x0C        // SPECIAL FUNCTION CONFIGURATION REGISTER
#define ADA4254_ERR_CFG                 0x0D        // ERROR CONFIGURATION REGISTER
#define ADA4254_TEST_MUX                0x0E        // TEST MULTIPLEXER REGISTER
#define ADA4254_EX_CURRENT_CFG          0x0F        // EXCITATION CURRENT CONFIGURATION REGISTER
#define ADA4254_GAIN_CAL_OFFSET         0x10        // GAIN CALIBRATION REGISTERS (0x10 to 0x27)
#define ADA4254_TRIG_CAL                0x2A        // TRIGGER CALIBRATION REGISTER
#define ADA4254_M_CLK_CNT               0x2E        // MASTER CLOCK COUNT REGISTER
#define ADA4254_DIE_REV_ID              0x2F        // DIE REVISION IDENTIFICATION REGISTER
#define ADA4254_PART_ID_OFFSET          0x64        // DEVICE IDENTIFICATION REGISTERS

#define CRC_DISABLED                    false
#define CRC_ENABLED                     true

/**
 * @brief Input gain values for ADA4254.
 */
typedef enum ada4254_in_gains_en
{
    IN_GAIN_0_0625  =   0u ,            // Input Gain 1/16
    IN_GAIN_0_125   =   1u ,            // Input Gain 1/8
    IN_GAIN_0_25    =   2u ,            // Input Gain 1/4
    IN_GAIN_0_5     =   3u ,            // Input Gain 1/2
    IN_GAIN_1       =   4u ,            // Input Gain 1
    IN_GAIN_2       =   5u ,            // Input Gain 2
    IN_GAIN_4       =   6u ,            // Input Gain 4
    IN_GAIN_8       =   7u ,            // Input Gain 8
    IN_GAIN_16      =   8u ,            // Input Gain 16
    IN_GAIN_32      =   9u ,            // Input Gain 32
    IN_GAIN_64      =   10u,            // Input Gain 64
    IN_GAIN_128     =   11u,            // Input Gain 128

} ada4254_in_gains_t;

/**
 * @brief Output gain values for ADA4254.
 */
typedef enum ada4254_out_gains_en
{
    OUT_GAIN_1      =   0u ,            // Output Gain 1
    OUT_GAIN_1_250  =   1u ,            // Output Gain 1.250
    OUT_GAIN_1_375  =   2u ,            // Output Gain 1.375

} ada4254_out_gains_t;

/**
 * @brief Input mux values for ADA4254.
 */
typedef enum ada4254_in_mux_en
{
    IN_MUX_CH1          =      0u,
    IN_MUX_CH2          =      1u,
    IN_MUX_TEST         =      2u,
    IN_MUX_SHORT        =      3u,

} ada4254_in_mux_t;

/**
 * @brief Union to handle analog errors.
 */
typedef union analog_err_un
{
    uint8_t  data;
    struct
    {
        uint8_t  MUX_OVER_VOLT_ERR  : 1;
        uint8_t  INPUT_ERR          : 1;
        uint8_t  OUTPUT_ERR         : 1;
        uint8_t  FAULT_INT          : 1;
        uint8_t  WB_ERR             : 1;
        uint8_t  reservd            : 1;
        uint8_t  POR_HV             : 1;
        uint8_t  G_RST              : 1;

    } bits;
} analog_err_t;


/**
 * @brief Union to handle digital errors.
 */
typedef union digital_err_un
{
    uint8_t  data;
    struct
    {
        uint8_t  ROM_CRC_ERR          : 1;
        uint8_t  MM_CRC_ERR           : 1;
        uint8_t  reservd1             : 1;
        uint8_t  SPI_SCLK_CNT_ERR     : 1;
        uint8_t  SPI_RW_ERR           : 1;
        uint8_t  SPI_CRC_ERR          : 1;
        uint8_t  CAL_BUSY             : 1;
        uint8_t  reservd0             : 1;

    } bits;
} digital_err_t;

/**
 * @brief Status codes for ADA4254.
 */
typedef enum ada4254_status_en
{
    ADA4254_STATUS_READ_ERROR           = -9,
    ADA4254_STATUS_GAIN_ERROR           = -8,
    ADA4254_STATUS_ANALOG_ERROR         = -7,
    ADA4254_STATUS_DIGITAL_ERROR        = -6,
    ADA4254_STATUS_WAIT_CAL_ERROR       = -5,
    ADA4254_STATUS_PERMANENT_ERROR      = -4,
    ADA4254_STATUS_CONN_ERROR           = -3,
    ADA4254_STATUS_CRC_ERROR            = -2,
    ADA4254_STATUS_ERROR                = -1,
    ADA4254_STATUS_NOT_INITIALIZED      = 0,
    ADA4254_STATUS_OK                   = 1,

} ada4254_status_t;

/**
 * @brief Structure to represent an ADA4254 channel.
 */
typedef struct ada4254_st {

    ada4254_status_t status;
    uint16_t  channel;                          // channel id
    uint16_t spi_base;                          // SPIA or SPIB
    uint16_t pin_cs;                            // gpio used as CS
    ada4254_in_gains_t input_gain;              // Input gain
    ada4254_out_gains_t output_gain;            // Output gain
    ada4254_in_mux_t input_mux;                 // Input mux
    uint8_t ext_mux;                            // External Mux (GPIO0 and GPIO1)
    analog_err_t  analog_err;                   // analogic error flags
    digital_err_t digital_err;                  // digital error flags
    float32_t f_gain;                           // Float gain considering Input and Output gains together
    float32_t inv_f_gain;                       // Inverse float gain to avoid unnecessary divisions
    //uint8_t gain_cal[24];
    bool    crc_on;
    uint8_t in_buffer[ADA4254_BUFFER_SIZE];
    uint8_t out_buffer[ADA4254_BUFFER_SIZE];


} ada4254_t;



enum { T_WAIT_AFTER_RESET_MS = 1,  //  ms
       T_WAIT_AFTER_VERIFY_MS = 5  // ms
};

typedef enum {
    INIT_ST_IDLE = 0,
    INIT_ST_READ_DIE,
    INIT_ST_DECIDE_RESET,
    INIT_ST_RESET,
    INIT_ST_WAIT_AFTER_RESET,
    INIT_ST_ENABLE_CRC,
    INIT_ST_TEST_CONN,
    INIT_ST_VERIFY,
    INIT_ST_WAIT_AFTER_VERIFY,
    INIT_ST_DONE,
    INIT_ST_ERROR
} ada4254_init_state_t;

typedef struct {
    ada4254_t     *ch;
    uint16_t       ch_id;
    uint16_t       spi_base;
    uint16_t       pin_cs;
    int            tries;
    ada4254_init_state_t st;
    uint32_t       deadline_us;
    ada4254_status_t last_status;
} ada4254_init_ctx_t;

typedef enum {
    ADA4254_GAIN_ST_IDLE = 0,
    ADA4254_GAIN_ST_SEND_CMD,
    ADA4254_GAIN_ST_WAIT_SETTLE,
    ADA4254_GAIN_ST_DONE,
    ADA4254_GAIN_ST_ERROR
} ada4254_gain_state_t;



typedef struct {
    ada4254_t* ch;
    uint32_t   deadline_us;
    ada4254_in_gains_t  in_gain;
    ada4254_out_gains_t out_gain;
    uint8_t    tries;
    ada4254_gain_state_t st;
} ada4254_gain_ctx_t;


/**
 * @brief Start for non blocking API
 *
 * @param
 */
void ada4254_init_start(ada4254_init_ctx_t *ctx, ada4254_t *channel,
                        uint16_t ch, uint16_t spi_base, uint16_t pin_cs);

ada4254_status_t ada4254_init_poll(ada4254_init_ctx_t *ctx, my_time_t now);


void ada4254_set_gain_start(ada4254_gain_ctx_t* gctx,
                            ada4254_t* ch,
                            ada4254_in_gains_t in_gain,
                            ada4254_out_gains_t out_gain);


/** Avança a máquina de estados de ganho.
 *  Retorna:
 *   ADA4254_STATUS_NOT_INITIALIZED  -> em progresso
 *   ADA4254_STATUS_OK               -> finalizado
 *   ADA4254_STATUS_READ_ERROR       -> erro recuperável/timeout
 *   ADA4254_STATUS_PERMANENT_ERROR  -> erro grave
 */
ada4254_status_t ada4254_set_gain_poll(ada4254_gain_ctx_t* gctx, my_time_t now);


/**
 * @brief Initializes the SPI module.
 *
 * @param base Base address of the SPI module.
 */
void SPI_init(uint16_t base);

/**
 * @brief Initializes the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param ch Channel to be initialized.
 * @param spi_base SPI base address.
 * @param pin_cs Chip select pin.
 * @return Status of the initialization.
 */
ada4254_status_t ada4254_init(ada4254_t *this_channel, uint16_t ch, uint16_t spi_base, uint16_t pin_cs);

/**
 * @brief Reads data from the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param addr Address to read from.
 * @param msg_size Number of bytes to read.
 * @return Status of the read operation.
 */
ada4254_status_t ada4254_read(ada4254_t *this_channel, uint8_t addr, uint8_t msg_size);

/**
 * @brief Writes data to the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param msg_size Number of bytes to write.
 * @return Status of the write operation.
 */
ada4254_status_t ada4254_write(ada4254_t *this_channel, uint8_t msg_size);

/**
 * @brief Sets the gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param in_gain Input gain.
 * @param out_gain Output gain.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_set_gain(ada4254_t *this_channel, ada4254_in_gains_t in_gain, ada4254_out_gains_t out_gain);

/**
 * @brief Reads the gain for the ADA4254 channel.
 * 
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_read_gain(ada4254_t * this_channel);

/**
 * @brief Configures CRC for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @param state CRC state (true to enable, false to disable).
 * @return Status of the operation.
 */
ada4254_status_t ada4254_crc_config(ada4254_t *this_channel, bool state);

/**
 * @brief Clears the input buffer for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_clear_input_buffer(ada4254_t *this_channel);

/**
 * @brief Tests the connection to the ADA4254 device.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the connection test.
 */
ada4254_status_t ada4254_test_conn(ada4254_t *this_channel);

/**
 * @brief Verifies the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the verification.
 */
ada4254_status_t ada4254_verify(ada4254_t *this_channel);

/**
 * @brief Resets the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 */
void ada4254_reset(ada4254_t *this_channel);

/**
 * @brief Increments the input gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_inc_in_gain(ada4254_t *this_channel);

/**
 * @brief Decrements the input gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_dec_in_gain(ada4254_t *this_channel);

/**
 * @brief Increments the output gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_inc_out_gain(ada4254_t *this_channel);

/**
 * @brief Decrements the output gain for the ADA4254 channel.
 *
 * @param this_channel Pointer to the ADA4254 channel structure.
 * @return Status of the operation.
 */
ada4254_status_t ada4254_dec_out_gain(ada4254_t *this_channel);


#endif /* ADA4254_H_ */

