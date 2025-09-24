/**
 * @file my_i2c.h
 * @brief Interface for I2C operations on the TMS320F28379D.
 */

#ifndef __MY_I2C_H_
#define __MY_I2C_H_

#include "device.h"
#include "i2c.h"


void I2CA_init(void);
void I2CA_kill(void);

uint16_t I2CA_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char *data, uint16_t data_size);
uint16_t I2CA_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size);


void gpio_i2c_init(uint32_t sda_pin, uint32_t sda_pin_cfg, uint32_t scl_pin, uint32_t scl_pin_cfg);

uint16_t I2CA_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char data[], uint16_t data_size);
uint16_t I2CA_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size);
uint16_t I2CA_WriteToManyRegs(uint16_t slave_addr, uint16_t reg[], uint16_t reg_size, uint16_t data[], uint16_t data_size);

void I2CA_init(void);
void I2CA_Program_ClkGen(void);


uint16_t I2CB_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char *data, uint16_t data_size);
uint16_t I2CB_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size);


void gpio_i2c_init(uint32_t sda_pin, uint32_t sda_pin_cfg, uint32_t scl_pin, uint32_t scl_pin_cfg);

uint16_t I2CB_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char data[], uint16_t data_size);
uint16_t I2CB_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size);
uint16_t I2CB_WriteToManyRegs(uint16_t slave_addr, uint16_t reg[], uint16_t reg_size, uint16_t data[], uint16_t data_size);

void I2CB_init(void);
void I2CB_Program_ClkGen(void);

#endif // __MY_I2C_H_

