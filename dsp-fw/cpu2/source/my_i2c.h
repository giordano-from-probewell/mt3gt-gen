#ifndef MY_I2C_H
#define MY_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "F2837xD_device.h"
#include "hw_types.h"


/**
 * @file my_i2c.h
 * @brief I2C driver interface for F2837xD microcontroller.
 */

/// I2C error codes
#define I2C_ERROR_NONE           0 /**< No error */
#define I2C_ERROR_NOT_INIT      -1 /**< I2C not initialized */
#define I2C_ERROR_BUSY          -2 /**< I2C bus busy */
#define I2C_ERROR_BUS           -3 /**< Bus error (NACK) */
#define I2C_ERROR_INVALID_ARG   -4 /**< Invalid argument */
#define I2C_ERROR_ALREADY_INIT  -5 /**< Already initialized */

/** Non-blocking I2C states */
typedef enum {
    I2C_STATE_IDLE,
    I2C_STATE_START,
    I2C_STATE_WAIT_START,
    I2C_STATE_SEND_REG_ADDR,
    I2C_STATE_RESTART,
    I2C_STATE_WAIT_RESTART,
    I2C_STATE_READ,
    I2C_STATE_WAIT_STOP,
    I2C_STATE_DONE,
    I2C_STATE_ERROR
} i2c_state_t;

/**
 * @brief Context for non-blocking I2C operations.
 */
typedef struct {
    volatile struct I2C_REGS *i2c_regs; /**< I2C registers */
    uint8_t device_address;            /**< 7-bit device address */
    bool request_start;                /**< Flag to initiate transaction */
    const uint8_t *write_data;         /**< Data to write */
    uint16_t write_length;             /**< Number of bytes to write */
    uint8_t *read_data;                /**< Buffer for read data */
    uint16_t read_length;              /**< Number of bytes to read */
    uint16_t write_index;              /**< Current write index */
    uint16_t read_index;               /**< Current read index */
    i2c_state_t state;                 /**< State machine state */
    int16_t result;                    /**< Last operation result */
    uint32_t timeout;                  /**< Timeout counter */
} i2c_context_t;

/**
 * @brief Initialize the I2C module.
 * @param regs Pointer to I2C registers.
 * @return I2C_ERROR_NONE on success or error code.
 */
int16 my_i2c_initiate(volatile struct I2C_REGS *i2c_regs);


/**
 * @brief Deinitialize the I2C module.
 * @param regs Pointer to I2C registers.
 * @return I2C_ERROR_NONE.
 */
int16 my_i2c_terminate(volatile struct I2C_REGS *i2c_regs);

/* Blocking API */
/**
 * @brief Read data from I2C device (blocking).
 * @param regs Pointer to I2C registers.
 * @param dev_addr 7-bit device address.
 * @param reg_addr Pointer to register address buffer.
 * @param reg_addr_cnt Number of register address bytes.
 * @param data Output buffer for read data.
 * @param cnt Number of bytes to read.
 * @return I2C_ERROR_NONE or error code.
 */
int16 my_i2c_read(volatile struct I2C_REGS *i2c_regs,
                  const uint8_t device_address,
                  const uint8_t *register_address,
                  const uint16_t register_address_count,
                  uint8_t *data, const uint16_t count);
/**
 * @brief Write data to I2C device (blocking).
 * @param regs Pointer to I2C registers.
 * @param dev_addr 7-bit device address.
 * @param reg_addr Pointer to register address buffer.
 * @param reg_addr_cnt Number of register address bytes.
 * @param data Data buffer to write.
 * @param cnt Number of bytes to write.
 * @return I2C_ERROR_NONE or error code.
 */
int16 my_i2c_write(volatile struct I2C_REGS *i2c_regs,
                   const uint8_t device_address,
                   const uint8_t *register_address,
                   const uint16_t register_address_count,
                   uint8_t *data, const uint16_t count);

/* Non-blocking API */
/**
 * @brief Start non-blocking write transaction.
 * @param ctx I2C context.
 */
void my_i2c_start_write(i2c_context_t *ctx);

/**
 * @brief Start non-blocking read transaction.
 * @param ctx I2C context.
 */
void my_i2c_start_read(i2c_context_t *ctx);

/**
 * @brief Process non-blocking I2C state machine.
 * @param ctx I2C context.
 */
void my_i2c_process(i2c_context_t *ctx);

#endif // MY_I2C_H
