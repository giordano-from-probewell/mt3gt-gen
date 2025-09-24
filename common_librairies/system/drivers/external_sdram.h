#ifndef EXTERNAL_SDRAM_H
#define EXTERNAL_SDRAM_H

#include <stdint.h>
#include "F28x_Project.h"

// Constants
#define TEST_PASS             0xABCDABCD
#define TEST_FAIL             0xDEADDEAD
#define SDRAM_CS0_START_ADDR  0x80000000
#define SDRAM_CS0_SIZE        0x300000

// Public variables
extern uint32_t TEST_STATUS;
extern uint16_t ErrCount;

// Interface functions
void InitEmif1Gpio(void);
void initEMIF1(void);
char sdram_data_walk(uint32_t start_addr, uint32_t mem_size);
char sdram_addr_walk(uint32_t start_addr, uint32_t addr_size);
char sdram_data_size(uint32_t start_addr, uint32_t mem_size);
char sdram_read_write(uint32_t start_addr, uint32_t mem_size);
void run_sdram_tests(void);

#endif // EXTERNAL_SDRAM_H
