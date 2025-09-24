/**
 * @file my_i2c.c
 * @brief Implementation of I2C operations on the TMS320F28379D.
 */

#include "my_i2c.h"
#include <stdbool.h>
#include "F28x_Project.h"
#include "gpio.h"
#include <stdio.h>
#include "my_time.h"


#define I2C_TIMEOUT_MS 100  /**< Timeout in milliseconds */
#define I2C_MAX_RETRIES 3    /**< Maximum retry count */

// Logging macros
#define LOG_INFO(...)    //printf(__VA_ARGS__)
#define LOG_ERROR(...)   //printf(__VA_ARGS__)



/**
 * @brief Resets the I2C bus.
 */
static void I2C_reset_bus(void)
{
    // Put the I2C module in reset
    I2caRegs.I2CMDR.bit.IRS = 0;
    // Wait a small delay
    DELAY_US(1000);
    // Release the I2C module from reset
    I2caRegs.I2CMDR.bit.IRS = 1;
    // Wait a small delay
    DELAY_US(1000);
}

void I2CA_init(void)
{

//    LOG_INFO("Initializing I2C...\n");
//    // Put the I2C module in reset
//    I2caRegs.I2CMDR.bit.IRS = 0;
//    // Wait a small delay
//    DELAY_US(2000);
//
//    I2caRegs.I2COAR.bit.OAR = 0;            // 7-bit addressing mode (XA = 0)
//    I2caRegs.I2CIER.all = 0;                // All interrupts disabled
//    I2caRegs.I2CCLKL = 0x03FF;
//    I2caRegs.I2CCLKH = 0x03FF;
//    I2caRegs.I2CCNT = 1;                    // Data count as 1 byte
//
//    I2caRegs.I2CMDR.bit.NACKMOD = 0;        // Sends an ACK when master-tx in cycle end
//    I2caRegs.I2CMDR.bit.FREE = 1;
//    I2caRegs.I2CMDR.bit.MST = 1;            // Master Mode On
//    I2caRegs.I2CMDR.bit.XA = 0;             // 7-Addressing mode
//    I2caRegs.I2CMDR.bit.RM = 0;             // Repeat mode: On
//    I2caRegs.I2CMDR.bit.DLB = 0;            // Digital Loopback: Off
//    I2caRegs.I2CMDR.bit.STB = 0;            // Not in start byte mode
//    I2caRegs.I2CMDR.bit.FDF = 0;            // Data with address
//    I2caRegs.I2CMDR.bit.BC = 0;             // 8-bit per data byte
//    // Wait a small delay
//    DELAY_US(2000);
//    I2caRegs.I2CMDR.bit.IRS = 1;            // Enable I2C Module
//    DELAY_US(2000);
//    LOG_INFO("I2C Initialized.\n");


    // Must put I2C into reset before configuring it
    I2C_disableModule(I2CA_BASE);

    // I2C configuration. Use a 100kHz I2CCLK with a 33% duty cycle.
    I2C_initMaster(I2CA_BASE, DEVICE_SYSCLK_FREQ, 100000, I2C_DUTYCYCLE_33);
    I2C_setBitCount(I2CA_BASE, I2C_BITCOUNT_8);
    //I2C_setSlaveAddress(I2CA_BASE, SLAVE_ADDRESS);
    I2C_setEmulationMode(I2CA_BASE, I2C_EMULATION_FREE_RUN);

    // Enable stop condition and register-access-ready interrupts
    //I2C_enableInterrupt(I2CA_BASE, I2C_INT_STOP_CONDITION |
    //                    I2C_INT_REG_ACCESS_RDY);

    // FIFO configuration
    I2C_enableFIFO(I2CA_BASE);
    I2C_clearInterruptStatus(I2CA_BASE, I2C_INT_RXFF | I2C_INT_TXFF);

    // Configuration complete. Enable the module.
    I2C_enableModule(I2CA_BASE);

}

void I2CA_kill(void)
{
    LOG_INFO("Terminating I2C...\n");
    DELAY_US(2000);
    I2caRegs.I2CMDR.bit.IRS = 0;
    DELAY_US(2000);
    LOG_INFO("I2C Terminated.\n");
}

//
// I2CA_ReadFromReg - This function reads data bytes from a slave
//                   device's register.
//
// INPUTS:
//      - slave_addr ==> Address of slave device being read from
//      - reg ==> Slave device register being read from
//      - data[] ==> data buffer to store the bytes read from the
//                      slave device register
//      - data_size ==> # of data bytes being received, size of data buffer
//


uint16_t I2CA_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char *data, uint16_t data_size)
{
    uint16_t i, Status;
    int error_counter=0;
    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    if (I2caRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2caRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // 1. Transmit the slave address followed by the register
    //      bytes being read from
    //

    //
    // Setup number of bytes to send
    // == # of register bytes
    //
    I2caRegs.I2CCNT = 1;

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2caRegs.I2CMDR.all = 0x4620;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition

    while(!I2caRegs.I2CSTR.bit.ARDY) // Wait for slave address to be sent
    {
        if(I2caRegs.I2CSTR.bit.XSMT == 0)
        {
            break;
        }
    }

    //    #if NACK_CHECK // check if NACK was received
    if(I2caRegs.I2CSTR.bit.NACK == 1)
    {
        I2caRegs.I2CMDR.bit.STP = 1;
        I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif



    // Transmit Register Byte
    error_counter = 0;
    while(!I2caRegs.I2CSTR.bit.XRDY){
        if(++error_counter>4000){
            Status = I2C_ERROR;
            return Status;
        }
    } // Make sure data
    // is ready to be written
    I2caRegs.I2CDXR.bit.DATA = reg;

    //    #if NACK_CHECK // check if NACK was received
    if(I2caRegs.I2CSTR.bit.NACK == 1)
    {
        I2caRegs.I2CMDR.bit.STP = 1;
        I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif


    // Wait for previous communication to complete
    error_counter = 0;
    while(!I2caRegs.I2CSTR.bit.ARDY){
        if(++error_counter>4000){
            Status = I2C_ERROR;
            return Status;
        }
    }

    //
    // 2. Receive data bytes from slave device
    //

    //
    // Set up as master receiver
    // FREE + MST + IRS ==> (Master Receiver)
    //
    I2caRegs.I2CMDR.all = 0x4420;

    //
    // Setup number of bytes to receive
    // == # of data bytes
    //
    I2caRegs.I2CCNT = data_size;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send repeated START condition & Slave Addr
    I2caRegs.I2CMDR.bit.STP = 0x1; // set STOP condition to be
    // generated when I2CCNT is zero

    //#if NACK_CHECK // check if NACK was received
    if(I2caRegs.I2CSTR.bit.NACK == 1)
    {
        I2caRegs.I2CMDR.bit.STP = 1;
        I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //#endif

    error_counter = 0;
    for (i=0; i< data_size; i++)
    {
        while(!I2caRegs.I2CSTR.bit.RRDY){
            if(++error_counter>4000){
                Status = I2C_ERROR;
                return Status;
            }} // Make sure data
        // is ready to be received

        data[i] = I2caRegs.I2CDRR.bit.DATA; // Buffer the received byte

    }

    // Data successfully read
    return Status;
}

uint16_t I2CA_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size)
{
    uint16_t i, Status;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
//        if (I2caRegs.I2CMDR.bit.STP == 1)
//        {
//            return I2C_STP_NOT_READY_ERROR;
//        }

    while(I2caRegs.I2CMDR.bit.STP){}

    //
    // Setup slave address
    //
    I2caRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
//        if (I2caRegs.I2CSTR.bit.BB == 1)
//        {
//            return I2C_BUS_BUSY_ERROR;
//        }

    while(I2caRegs.I2CSTR.bit.BB){}

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2caRegs.I2CMDR.all = 0x4620;

    //
    // Setup number of bytes to send
    // == register byte + (# of data[] buffer bytes)
    //
    I2caRegs.I2CCNT = 1 + data_size;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    I2caRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
    // generated when I2CCNT is zero

    //
    // I2C module will send the following:
    // register byte ==> data bytes ==> STOP condition
    //
    while(!I2caRegs.I2CSTR.bit.XRDY){} // Make sure data
    // is ready to be written
    I2caRegs.I2CDXR.bit.DATA = reg;

    //    #if NACK_CHECK // check if NACK was received
    if(I2caRegs.I2CSTR.bit.NACK == 1)
    {
        I2caRegs.I2CMDR.bit.STP = 1;
        I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif

    // Transmit Data Bytes followed by STOP condition
    for (i=0; i< data_size; i++)
    {
        while(!I2caRegs.I2CSTR.bit.XRDY){} // Make sure data
        // is ready to be written
        I2caRegs.I2CDXR.bit.DATA = data[i];

        //        #if NACK_CHECK // check if NACK was received
        if(I2caRegs.I2CSTR.bit.NACK == 1)
        {
            I2caRegs.I2CMDR.bit.STP = 1;
            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

            Status = I2C_ERROR;
            break;
        }
        //        #endif
    }


    // Data successfully written
    return Status;
}

uint16_t I2CA_WriteToManyRegs(uint16_t slave_addr, uint16_t reg[],
               uint16_t reg_size, uint16_t data[], uint16_t data_size)
{
    uint16_t i, Status;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    if (I2caRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2caRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2caRegs.I2CMDR.all = 0x4620;

    //
    // Setup number of bytes to send
    // == (# of register bytes) + (# of data[] buffer bytes)
    //
    I2caRegs.I2CCNT = reg_size + data_size;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    I2caRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
                                   // generated when I2CCNT is zero

    //
    // I2C module will send the following:
    // register bytes ==> data bytes ==> STOP condition
    //

    // Transmit Register Bytes
    for (i=0; i< reg_size; i++)
    {
        while(!I2caRegs.I2CSTR.bit.XRDY){} // Make sure data
                                           // is ready to be written
        I2caRegs.I2CDXR.bit.DATA = reg[i];

//        #if NACK_CHECK // check if NACK was received
            if(I2caRegs.I2CSTR.bit.NACK == 1)
            {
                I2caRegs.I2CMDR.bit.STP = 1;
                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
//        #endif
    }

    // Transmit Data Bytes followed by STOP condition
    for (i=0; i< data_size; i++)
    {
        while(!I2caRegs.I2CSTR.bit.XRDY){} // Make sure data
                                           // is ready to be written
        I2caRegs.I2CDXR.bit.DATA = data[i];

//        #if NACK_CHECK // check if NACK was received
            if(I2caRegs.I2CSTR.bit.NACK == 1)
            {
                I2caRegs.I2CMDR.bit.STP = 1;
                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
//        #endif
    }


    // Data successfully written
    return Status;
}










//////////////////////////////////////////////









/**
 * @brief Resets the I2C bus.
 */
static void I2CB_reset_bus(void)
{
    // Put the I2C module in reset
    I2cbRegs.I2CMDR.bit.IRS = 0;
    // Wait a small delay
    DELAY_US(1000);
    // Release the I2C module from reset
    I2cbRegs.I2CMDR.bit.IRS = 1;
    // Wait a small delay
    DELAY_US(1000);
}

void I2CB_init(void)
{
//    LOG_INFO("Initializing I2C...\n");
//    DELAY_US(2000);
//    // Put the I2C module in reset
//    I2cbRegs.I2CMDR.bit.IRS = 0;
//    // Wait a small delay
//    DELAY_US(2000);
//
//    I2cbRegs.I2COAR.bit.OAR = 0;            // 7-bit addressing mode (XA = 0)
//    I2cbRegs.I2CIER.all = 0;                // All interrupts disabled
//    I2cbRegs.I2CCLKL = 0x03FF;
//    I2cbRegs.I2CCLKH = 0x03FF;
//    I2cbRegs.I2CCNT = 1;                    // Data count as 1 byte
//    I2cbRegs.I2CMDR.bit.NACKMOD = 0;        // Sends an ACK when master-tx in cycle end
//    I2cbRegs.I2CMDR.bit.FREE = 1;
//    I2cbRegs.I2CMDR.bit.MST = 1;            // Master Mode On
//    I2cbRegs.I2CMDR.bit.XA = 0;             // 7-Addressing mode
//    I2cbRegs.I2CMDR.bit.RM = 0;             // Repeat mode: On
//    I2cbRegs.I2CMDR.bit.DLB = 0;            // Digital Loopback: Off
//    I2cbRegs.I2CMDR.bit.STB = 0;            // Not in start byte mode
//    I2cbRegs.I2CMDR.bit.FDF = 0;            // Data with address
//    I2cbRegs.I2CMDR.bit.BC = 0;             // 8-bit per data byte
//    // Wait a small delay
//    DELAY_US(2000);
//    I2cbRegs.I2CMDR.bit.IRS = 1;            // Enable I2C Module
//    DELAY_US(2000);
//    LOG_INFO("I2C Initialized.\n");



    // Must put I2C into reset before configuring it
    I2C_disableModule(I2CB_BASE);

    // I2C configuration. Use a 100kHz I2CCLK with a 33% duty cycle.
    I2C_initMaster(I2CB_BASE, DEVICE_SYSCLK_FREQ, 100000, I2C_DUTYCYCLE_33);
    I2C_setBitCount(I2CB_BASE, I2C_BITCOUNT_8);
    //I2C_setSlaveAddress(I2CA_BASE, SLAVE_ADDRESS);
    I2C_setEmulationMode(I2CB_BASE, I2C_EMULATION_FREE_RUN);

    // Enable stop condition and register-access-ready interrupts

//    I2C_enableInterrupt(I2CB_BASE, I2C_INT_STOP_CONDITION |
//                        I2C_INT_REG_ACCESS_RDY);

    // FIFO configuration
    I2C_enableFIFO(I2CB_BASE);
    I2C_clearInterruptStatus(I2CB_BASE, I2C_INT_RXFF | I2C_INT_TXFF);

    // Configuration complete. Enable the module.
    I2C_enableModule(I2CB_BASE);


}

void I2CB_kill(void)
{
    LOG_INFO("Terminating I2C...\n");
    DELAY_US(2000);
    I2cbRegs.I2CMDR.bit.IRS = 0;
    DELAY_US(2000);
    LOG_INFO("I2C Terminated.\n");
}

//
// I2CA_ReadFromReg - This function reads data bytes from a slave
//                   device's register.
//
// INPUTS:
//      - slave_addr ==> Address of slave device being read from
//      - reg ==> Slave device register being read from
//      - data[] ==> data buffer to store the bytes read from the
//                      slave device register
//      - data_size ==> # of data bytes being received, size of data buffer
//


uint16_t I2CB_ReadFromReg(uint16_t slave_addr, uint16_t reg, unsigned char *data, uint16_t data_size)
{
    uint16_t i, Status;
    int error_counter=0;
    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    if (I2cbRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2cbRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2cbRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // 1. Transmit the slave address followed by the register
    //      bytes being read from
    //

    //
    // Setup number of bytes to send
    // == # of register bytes
    //
    I2cbRegs.I2CCNT = 1;

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2cbRegs.I2CMDR.all = 0x4620;

    I2cbRegs.I2CMDR.bit.STT = 0x1; // Send START condition

    while(!I2cbRegs.I2CSTR.bit.ARDY) // Wait for slave address to be sent
    {
        if(I2cbRegs.I2CSTR.bit.XSMT == 0)
        {
            break;
        }
    }

    //    #if NACK_CHECK // check if NACK was received
    if(I2cbRegs.I2CSTR.bit.NACK == 1)
    {
        I2cbRegs.I2CMDR.bit.STP = 1;
        I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif



    // Transmit Register Byte
    error_counter = 0;
    while(!I2cbRegs.I2CSTR.bit.XRDY){
        if(++error_counter>4000){
            Status = I2C_ERROR;
            return Status;
        }
    } // Make sure data
    // is ready to be written
    I2cbRegs.I2CDXR.bit.DATA = reg;

    //    #if NACK_CHECK // check if NACK was received
    if(I2cbRegs.I2CSTR.bit.NACK == 1)
    {
        I2cbRegs.I2CMDR.bit.STP = 1;
        I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif


    // Wait for previous communication to complete
    error_counter = 0;
    while(!I2cbRegs.I2CSTR.bit.ARDY){
        if(++error_counter>4000){
            Status = I2C_ERROR;
            return Status;
        }
    }

    //
    // 2. Receive data bytes from slave device
    //

    //
    // Set up as master receiver
    // FREE + MST + IRS ==> (Master Receiver)
    //
    I2cbRegs.I2CMDR.all = 0x4420;

    //
    // Setup number of bytes to receive
    // == # of data bytes
    //
    I2cbRegs.I2CCNT = data_size;

    I2cbRegs.I2CMDR.bit.STT = 0x1; // Send repeated START condition & Slave Addr
    I2cbRegs.I2CMDR.bit.STP = 0x1; // set STOP condition to be
    // generated when I2CCNT is zero

    //#if NACK_CHECK // check if NACK was received
    if(I2cbRegs.I2CSTR.bit.NACK == 1)
    {
        I2cbRegs.I2CMDR.bit.STP = 1;
        I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //#endif

    error_counter = 0;
    for (i=0; i< data_size; i++)
    {
        while(!I2cbRegs.I2CSTR.bit.RRDY){
            if(++error_counter>4000){
                Status = I2C_ERROR;
                return Status;
            }} // Make sure data
        // is ready to be received

        data[i] = I2cbRegs.I2CDRR.bit.DATA; // Buffer the received byte

    }

    // Data successfully read
    return Status;
}

uint16_t I2CB_WriteToReg(uint16_t slave_addr, uint16_t reg, const unsigned char *data, uint16_t data_size)
{
    uint16_t i, Status;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
//        if (I2cbRegs.I2CMDR.bit.STP == 1)
//        {
//            return I2C_STP_NOT_READY_ERROR;
//        }

    while(I2cbRegs.I2CMDR.bit.STP){}

    //
    // Setup slave address
    //
    I2cbRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
//        if (I2cbRegs.I2CSTR.bit.BB == 1)
//        {
//            return I2C_BUS_BUSY_ERROR;
//        }

    while(I2cbRegs.I2CSTR.bit.BB){}

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2cbRegs.I2CMDR.all = 0x4620;

    //
    // Setup number of bytes to send
    // == register byte + (# of data[] buffer bytes)
    //
    I2cbRegs.I2CCNT = 1 + data_size;

    I2cbRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    I2cbRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
    // generated when I2CCNT is zero

    //
    // I2C module will send the following:
    // register byte ==> data bytes ==> STOP condition
    //
    while(!I2cbRegs.I2CSTR.bit.XRDY){} // Make sure data
    // is ready to be written
    I2cbRegs.I2CDXR.bit.DATA = reg;

    //    #if NACK_CHECK // check if NACK was received
    if(I2cbRegs.I2CSTR.bit.NACK == 1)
    {
        I2cbRegs.I2CMDR.bit.STP = 1;
        I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

        Status = I2C_ERROR;
        return Status;
    }
    //    #endif

    // Transmit Data Bytes followed by STOP condition
    for (i=0; i< data_size; i++)
    {
        while(!I2cbRegs.I2CSTR.bit.XRDY){} // Make sure data
        // is ready to be written
        I2cbRegs.I2CDXR.bit.DATA = data[i];

        //        #if NACK_CHECK // check if NACK was received
        if(I2cbRegs.I2CSTR.bit.NACK == 1)
        {
            I2cbRegs.I2CMDR.bit.STP = 1;
            I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

            Status = I2C_ERROR;
            break;
        }
        //        #endif
    }


    // Data successfully written
    return Status;
}

uint16_t I2CB_WriteToManyRegs(uint16_t slave_addr, uint16_t reg[],
               uint16_t reg_size, uint16_t data[], uint16_t data_size)
{
    uint16_t i, Status;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    if (I2cbRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2cbRegs.I2CSAR.bit.SAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2cbRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2cbRegs.I2CMDR.all = 0x4620;

    //
    // Setup number of bytes to send
    // == (# of register bytes) + (# of data[] buffer bytes)
    //
    I2cbRegs.I2CCNT = reg_size + data_size;

    I2cbRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    I2cbRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
                                   // generated when I2CCNT is zero

    //
    // I2C module will send the following:
    // register bytes ==> data bytes ==> STOP condition
    //

    // Transmit Register Bytes
    for (i=0; i< reg_size; i++)
    {
        while(!I2cbRegs.I2CSTR.bit.XRDY){} // Make sure data
                                           // is ready to be written
        I2cbRegs.I2CDXR.bit.DATA = reg[i];

//        #if NACK_CHECK // check if NACK was received
            if(I2cbRegs.I2CSTR.bit.NACK == 1)
            {
                I2cbRegs.I2CMDR.bit.STP = 1;
                I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
//        #endif
    }

    // Transmit Data Bytes followed by STOP condition
    for (i=0; i< data_size; i++)
    {
        while(!I2cbRegs.I2CSTR.bit.XRDY){} // Make sure data
                                           // is ready to be written
        I2cbRegs.I2CDXR.bit.DATA = data[i];

//        #if NACK_CHECK // check if NACK was received
            if(I2cbRegs.I2CSTR.bit.NACK == 1)
            {
                I2cbRegs.I2CMDR.bit.STP = 1;
                I2cbRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
//        #endif
    }


    // Data successfully written
    return Status;
}

