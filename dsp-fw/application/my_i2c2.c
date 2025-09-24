#include "my_i2c2.h"
#include "driverlib.h"
#include "device.h"
#include "application.h"

//
// Defines
//
#define EEPROM_ADDR                 0x50
#define SI5351C_ADDR                0x60
#define EEPROM_HIGH_ADDR            0x00
#define EEPROM_LOW_ADDR             0x30
#define NUM_BYTES                   8
#define MAX_BUFFER_SIZE             14      // Max is currently 14 because of
// 2 address bytes and the 16-byte
// FIFO

//
// I2C message states for I2CMsg struct
//
#define MSG_STATUS_INACTIVE         0x0000 // Message not in use, do not send
#define MSG_STATUS_SEND_WITHSTOP    0x0010 // Send message with stop bit
#define MSG_STATUS_WRITE_BUSY       0x0011 // Message sent, wait for stop
#define MSG_STATUS_SEND_NOSTOP      0x0020 // Send message without stop bit
#define MSG_STATUS_SEND_NOSTOP_BUSY 0x0021 // Message sent, wait for ARDY
#define MSG_STATUS_RESTART          0x0022 // Ready to become master-receiver
#define MSG_STATUS_READ_BUSY        0x0023 // Wait for stop before reading data

//
// Error messages for read and write functions
//
#define ERROR_BUS_BUSY              0x1000
#define ERROR_STOP_NOT_READY        0x5555
#define SUCCESS                     0x0000

//
// Typedefs
//
struct I2CMsg
{
    uint16_t msgStatus;                  // Word stating what state msg is in.
    // See MSG_STATUS_* defines above.
    uint16_t slaveAddr;                  // Slave address tied to the message.
    uint16_t numBytes;                   // Number of valid bytes in message.
    uint16_t memoryHighAddr;             // EEPROM address of data associated
    // with message (high byte).
    uint16_t memoryLowAddr;              // EEPROM address of data associated
    // with message (low byte).
    uint16_t msgBuffer[MAX_BUFFER_SIZE]; // Array holding message data.
};

//
// Globals
//
struct I2CMsg i2cMsgOut = {MSG_STATUS_SEND_WITHSTOP,
                           EEPROM_ADDR,
                           NUM_BYTES,
                           EEPROM_HIGH_ADDR,
                           EEPROM_LOW_ADDR,
                           0x01,                // Message bytes
                           0x23,
                           0x45,
                           0x67,
                           0x89,
                           0xAB,
                           0xCD,
                           0xEF};
struct I2CMsg i2cMsgIn  = {MSG_STATUS_SEND_NOSTOP,
                           EEPROM_ADDR,
                           NUM_BYTES,
                           EEPROM_HIGH_ADDR,
                           EEPROM_LOW_ADDR};

struct I2CMsg *currentMsgPtr;                   // Used in interrupt

uint16_t passCount = 0;
uint16_t failCount = 0;

//
// Function Prototypes
//
void initI2C(void);
uint16_t readData(struct I2CMsg *msg);
uint16_t writeData1(struct I2CMsg *msg);
uint16_t writeData2(struct I2CMsg *msg);




__interrupt void i2cBISR(void);


void eeprom_test(void)
{
    uint16_t error;
    uint16_t i;



    //
    // Interrupts that are used in this example are re-mapped to ISR functions
    // found within this file.
    //
    Interrupt_register(INT_I2CB, &i2cBISR);

    //
    // Set I2C use, initializing it for FIFO mode
    //
    initI2C();

    //
    // Clear incoming message buffer
    //
    for (i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        i2cMsgIn.msgBuffer[i] = 0x0000;
    }

    //
    // Set message pointer used in interrupt to point to outgoing message
    //
    currentMsgPtr = &i2cMsgOut;

    //
    // Enable interrupts required for this example
    //
    Interrupt_enable(INT_I2CB);

    //
    // Loop indefinitely
    //
    while(1)
    {
        //
        // **** Write data to EEPROM section ****
        //
        // Check the outgoing message to see if it should be sent. In this
        // example it is initialized to send with a stop bit.
        //
        if(i2cMsgOut.msgStatus == MSG_STATUS_SEND_WITHSTOP)
        {
            //
            // Send the data to the EEPROM
            //
            error = writeData2(&i2cMsgOut);

            //
            // If communication is correctly initiated, set msg status to busy
            // and update currentMsgPtr for the interrupt service routine.
            // Otherwise, do nothing and try again next loop. Once message is
            // initiated, the I2C interrupts will handle the rest. See the
            // function i2cBISR().
            //
            if(error == SUCCESS)
            {
                currentMsgPtr = &i2cMsgOut;
                i2cMsgOut.msgStatus = MSG_STATUS_WRITE_BUSY;
            }
        }

        //
        // **** Read data from EEPROM section ****
        //
        // Check outgoing message status. Bypass read section if status is
        // not inactive.
        //
        if (i2cMsgOut.msgStatus == MSG_STATUS_INACTIVE)
        {
            //
            // Check incoming message status
            //
            if(i2cMsgIn.msgStatus == MSG_STATUS_SEND_NOSTOP)
            {
                //
                // Send EEPROM address setup
                //
                while(readData(&i2cMsgIn) != SUCCESS)
                {
                    //
                    // Maybe setup an attempt counter to break an infinite
                    // while loop. The EEPROM will send back a NACK while it is
                    // performing a write operation. Even though the write
                    // is complete at this point, the EEPROM could still be
                    // busy programming the data. Therefore, multiple
                    // attempts are necessary.
                    //
                }

                //
                // Update current message pointer and message status
                //
                currentMsgPtr = &i2cMsgIn;
                i2cMsgIn.msgStatus = MSG_STATUS_SEND_NOSTOP_BUSY;
            }

            //
            // Once message has progressed past setting up the internal address
            // of the EEPROM, send a restart to read the data bytes from the
            // EEPROM. Complete the communique with a stop bit. msgStatus is
            // updated in the interrupt service routine.
            //
            else if(i2cMsgIn.msgStatus == MSG_STATUS_RESTART)
            {
                //
                // Read data portion
                //
                while(readData(&i2cMsgIn) != SUCCESS)
                {
                    //
                    // Maybe setup an attempt counter to break an infinite
                    // while loop.
                    //
                }

                //
                // Update current message pointer and message status
                //
                currentMsgPtr = &i2cMsgIn;
                i2cMsgIn.msgStatus = MSG_STATUS_READ_BUSY;
            }
        }
    }
}

void clk_test(void)
{
    uint16_t error;
    uint16_t i;




    //
    // Interrupts that are used in this example are re-mapped to ISR functions
    // found within this file.
    //
    Interrupt_register(INT_I2CB, &i2cBISR);

    //
    // Set I2C use, initializing it for FIFO mode
    //
    initI2C();

    //
    // Clear incoming message buffer
    //
    for (i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        i2cMsgIn.msgBuffer[i] = 0x0000;
    }

    //
    // Set message pointer used in interrupt to point to outgoing message
    //
    currentMsgPtr = &i2cMsgOut;

    //
    // Enable interrupts required for this example
    //
    Interrupt_enable(INT_I2CB);

    i2cMsgOut.slaveAddr = SI5351C_ADDR;
    i2cMsgOut.numBytes = 1;
    i2cMsgOut.memoryLowAddr = 0x03;
    i2cMsgOut.msgBuffer[0] = 0xFF;
    error = writeData1(&i2cMsgOut);

    //
    // If communication is correctly initiated, set msg status to busy
    // and update currentMsgPtr for the interrupt service routine.
    // Otherwise, do nothing and try again next loop. Once message is
    // initiated, the I2C interrupts will handle the rest. See the
    // function i2cBISR().
    //
    if(error == SUCCESS)
    {
        currentMsgPtr = &i2cMsgOut;
        i2cMsgOut.msgStatus = MSG_STATUS_WRITE_BUSY;
    }

    //while(i2cMsgOut.msgStatus = MSG_STATUS_WRITE_BUSY);

    i2cMsgOut.slaveAddr = SI5351C_ADDR;
    i2cMsgOut.numBytes = 8;
    i2cMsgOut.memoryLowAddr = 0x10;
    i2cMsgOut.msgBuffer[0] = 0x8;
    i2cMsgOut.msgBuffer[2] = 0x8;
    i2cMsgOut.msgBuffer[3] = 0x8;
    i2cMsgOut.msgBuffer[4] = 0x8;
    i2cMsgOut.msgBuffer[5] = 0x8;
    i2cMsgOut.msgBuffer[6] = 0x8;
    i2cMsgOut.msgBuffer[7] = 0x8;
    i2cMsgOut.msgBuffer[8] = 0x8;
    error = writeData1(&i2cMsgOut);



}


//
// initI2C - Function to configure I2C A in FIFO mode.
//
void
initI2C()
{
    //
    // Must put I2C into reset before configuring it
    //
    I2C_disableModule(I2CB_BASE);

    //
    // I2C configuration. Use a 400kHz I2CCLK with a 33% duty cycle.
    //
    I2C_initMaster(I2CB_BASE, DEVICE_SYSCLK_FREQ, 100000, I2C_DUTYCYCLE_33);
    I2C_setBitCount(I2CB_BASE, I2C_BITCOUNT_8);
    I2C_setSlaveAddress(I2CB_BASE, EEPROM_ADDR);
    I2C_setEmulationMode(I2CB_BASE, I2C_EMULATION_FREE_RUN);

    //
    // Enable stop condition and register-access-ready interrupts
    //
    I2C_enableInterrupt(I2CB_BASE, I2C_INT_STOP_CONDITION |
                        I2C_INT_REG_ACCESS_RDY);

    //
    // FIFO configuration
    //
    I2C_enableFIFO(I2CB_BASE);
    I2C_clearInterruptStatus(I2CB_BASE, I2C_INT_RXFF | I2C_INT_TXFF);

    //
    // Configuration complete. Enable the module.
    //
    I2C_enableModule(I2CB_BASE);
}

//
// writeData - Function to send the data that is to be written to the EEPROM
//
uint16_t
writeData2(struct I2CMsg *msg)
{
    uint16_t i;

    //
    // Wait until the STP bit is cleared from any previous master
    // communication. Clearing of this bit by the module is delayed until after
    // the SCD bit is set. If this bit is not checked prior to initiating a new
    // message, the I2C could get confused.
    //
    if(I2C_getStopConditionStatus(I2CB_BASE))
    {
        return(ERROR_STOP_NOT_READY);
    }

    //
    // Setup slave address
    //
    I2C_setSlaveAddress(I2CB_BASE, msg->slaveAddr);

    //
    // Check if bus busy
    //
    if(I2C_isBusBusy(I2CB_BASE))
    {
        return(ERROR_BUS_BUSY);
    }

    //
    // Setup number of bytes to send msgBuffer and address
    //
    I2C_setDataCount(I2CB_BASE, (msg->numBytes + 2));

    //
    // Setup data to send
    //
    I2C_putData(I2CB_BASE, msg->memoryHighAddr);
    I2C_putData(I2CB_BASE, msg->memoryLowAddr);

    for (i = 0; i < msg->numBytes; i++)
    {
        I2C_putData(I2CB_BASE, msg->msgBuffer[i]);
    }

    //
    // Send start as master transmitter
    //
    I2C_setConfig(I2CB_BASE, I2C_MASTER_SEND_MODE);
    I2C_sendStartCondition(I2CB_BASE);
    I2C_sendStopCondition(I2CB_BASE);

    return(SUCCESS);
}

//
// writeData - Function to send the data that is to be written to the EEPROM
//
uint16_t
writeData1(struct I2CMsg *msg)
{
    uint16_t i;

    //
    // Wait until the STP bit is cleared from any previous master
    // communication. Clearing of this bit by the module is delayed until after
    // the SCD bit is set. If this bit is not checked prior to initiating a new
    // message, the I2C could get confused.
    //
    if(I2C_getStopConditionStatus(I2CB_BASE))
    {
        return(ERROR_STOP_NOT_READY);
    }

    //
    // Setup slave address
    //
    I2C_setSlaveAddress(I2CB_BASE, msg->slaveAddr);

    //
    // Check if bus busy
    //
    if(I2C_isBusBusy(I2CB_BASE))
    {
        return(ERROR_BUS_BUSY);
    }

    //
    // Setup number of bytes to send msgBuffer and address
    //
    I2C_setDataCount(I2CB_BASE, (msg->numBytes + 2));

    //
    // Setup data to send
    //
    I2C_putData(I2CB_BASE, msg->memoryLowAddr);

    for (i = 0; i < msg->numBytes; i++)
    {
        I2C_putData(I2CB_BASE, msg->msgBuffer[i]);
    }

    //
    // Send start as master transmitter
    //
    I2C_setConfig(I2CB_BASE, I2C_MASTER_SEND_MODE);
    I2C_sendStartCondition(I2CB_BASE);
    I2C_sendStopCondition(I2CB_BASE);

    return(SUCCESS);
}


//
// readData - Function to prepare for the data that is to be read from the EEPROM
//
uint16_t
readData(struct I2CMsg *msg)
{
    //
    // Wait until the STP bit is cleared from any previous master
    // communication. Clearing of this bit by the module is delayed until after
    // the SCD bit is set. If this bit is not checked prior to initiating a new
    // message, the I2C could get confused.
    //
    if(I2C_getStopConditionStatus(I2CB_BASE))
    {
        return(ERROR_STOP_NOT_READY);
    }

    //
    // Setup slave address
    //
    I2C_setSlaveAddress(I2CB_BASE, msg->slaveAddr);

    //
    // If we are in the the address setup phase, send the address without a
    // stop condition.
    //
    if(msg->msgStatus == MSG_STATUS_SEND_NOSTOP)
    {
        //
        // Check if bus busy
        //
        if(I2C_isBusBusy(I2CB_BASE))
        {
            return(ERROR_BUS_BUSY);
        }

        //
        // Send data to setup EEPROM address
        //
        I2C_setDataCount(I2CB_BASE, 2);
        I2C_putData(I2CB_BASE, msg->memoryHighAddr);
        I2C_putData(I2CB_BASE, msg->memoryLowAddr);
        I2C_setConfig(I2CB_BASE, I2C_MASTER_SEND_MODE);
        I2C_sendStartCondition(I2CB_BASE);
    }
    else if(msg->msgStatus == MSG_STATUS_RESTART)
    {
        //
        // Address setup phase has completed. Now setup how many bytes expected
        // and send restart as master-receiver.
        //
        I2C_setDataCount(I2CB_BASE, (msg->numBytes));
        I2C_setConfig(I2CB_BASE, I2C_MASTER_RECEIVE_MODE);
        I2C_sendStartCondition(I2CB_BASE);
        I2C_sendStopCondition(I2CB_BASE);
    }

    return(SUCCESS);
}

//
// i2cBISR - I2C B ISR (non-FIFO)
//
__interrupt void
i2cBISR(void)
{
    I2C_InterruptSource intSource;
    uint16_t i;

    //
    // Read interrupt source
    //
    intSource = I2C_getInterruptSource(I2CB_BASE);

    //
    // Interrupt source = stop condition detected
    //
    if(intSource == I2C_INTSRC_STOP_CONDITION)
    {
        //
        // If completed message was writing data, reset msg to inactive state
        //
        if(currentMsgPtr->msgStatus == MSG_STATUS_WRITE_BUSY)
        {
            currentMsgPtr->msgStatus = MSG_STATUS_INACTIVE;
        }
        else
        {
            //
            // If a message receives a NACK during the address setup portion of
            // the EEPROM read, the code further below included in the register
            // access ready interrupt source code will generate a stop
            // condition. After the stop condition is received (here), set the
            // message status to try again. User may want to limit the number
            // of retries before generating an error.
            //
            if(currentMsgPtr->msgStatus == MSG_STATUS_SEND_NOSTOP_BUSY)
            {
                currentMsgPtr->msgStatus = MSG_STATUS_SEND_NOSTOP;
            }
            //
            // If completed message was reading EEPROM data, reset message to
            // inactive state and read data from FIFO.
            //
            else if(currentMsgPtr->msgStatus == MSG_STATUS_READ_BUSY)
            {
                currentMsgPtr->msgStatus = MSG_STATUS_INACTIVE;
                for(i=0; i < NUM_BYTES; i++)
                {
                    currentMsgPtr->msgBuffer[i] = I2C_getData(I2CB_BASE);
                }

                //
                // Check received data
                //
                for(i=0; i < NUM_BYTES; i++)
                {
                    if(i2cMsgIn.msgBuffer[i] == i2cMsgOut.msgBuffer[i])
                    {
                        passCount++;
                    }
                    else
                    {
                        failCount++;
                    }
                }

                //                if(passCount == NUM_BYTES)
                //                {
                //                    pass();        asm("   ESTOP0");
                //                }
                //                else
                //                {
                //                    fail();        asm("   ESTOP0");
                //                }
            }
        }
    }
    //
    // Interrupt source = Register Access Ready
    //
    // This interrupt is used to determine when the EEPROM address setup
    // portion of the read data communication is complete. Since no stop bit
    // is commanded, this flag tells us when the message has been sent
    // instead of the SCD flag.
    //
    else if(intSource == I2C_INTSRC_REG_ACCESS_RDY)
    {
        //
        // If a NACK is received, clear the NACK bit and command a stop.
        // Otherwise, move on to the read data portion of the communication.
        //
        if((I2C_getStatus(I2CB_BASE) & I2C_STS_NO_ACK) != 0)
        {
            I2C_sendStopCondition(I2CB_BASE);
            I2C_clearStatus(I2CB_BASE, I2C_STS_NO_ACK);
        }
        else if(currentMsgPtr->msgStatus == MSG_STATUS_SEND_NOSTOP_BUSY)
        {
            currentMsgPtr->msgStatus = MSG_STATUS_RESTART;
        }
    }
    else
    {
        //
        // Generate some error from invalid interrupt source
        //
        asm("   ESTOP0");
    }

    //
    // Issue ACK to enable future group 8 interrupts
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP8);
}


//
// End of File
//

