#include "commands.h"



void send_command_to_CPU1(uint32_t command, float32_t payload) {
    static uint32_t commandFlag = 0;

    // Write command idetifier in shared memory
    *(volatile uint32_t *)IPC_CMD_ADDR = command;

    // Write payload command in shared memory
    *(volatile float32_t *)IPC_PAYLOAD_ADDR = payload;

    // Alternate flag to indicate new command
    commandFlag = commandFlag + 1;
    *(volatile uint32_t *)IPC_FLAG_ADDR = commandFlag;
}
