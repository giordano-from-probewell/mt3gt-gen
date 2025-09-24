#ifndef CRC_UTILS_H
#define CRC_UTILS_H

#include <stdint.h>
#include "_comm_internals.h"

#define CRC_POLY 0x1021
#define CRC_INIT 0xFFFF

/**
 * Update the CRC-16 value with a new byte.
 *
 * @param crc  The current CRC-16 value
 * @param byte The new byte to include in the CRC calculation
 * @return     The updated CRC-16 value
 */
static inline uint16_t update_crc16(uint16_t crc, uint8_t byte)
{
    uint8_t j;
    crc ^= (byte << 8);
    for (j = 0; j < 8; j++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ CRC_POLY;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

/**
 * Calculate CRC-16 for a given packet.
 *
 * @param packet Pointer to the packet structure
 * @return       The calculated CRC-16 value
 */
static inline uint16_t calculate_crc16(const _packet_cmd_t *packet)
{
    uint16_t crc = CRC_INIT;
    uint16_t i;
    uint8_t bytes[2];

    // Process preamble
    bytes[0] = (uint8_t)(packet->preamble >> 8);
    bytes[1] = (uint8_t)(packet->preamble & 0xFF);
    crc = update_crc16(crc, bytes[0]);
    crc = update_crc16(crc, bytes[1]);

    // Process command
    bytes[0] = (uint8_t)(packet->command.value >> 8);
    bytes[1] = (uint8_t)(packet->command.value & 0xFF);
    crc = update_crc16(crc, bytes[0]);
    crc = update_crc16(crc, bytes[1]);

    // Process size
    bytes[0] = (uint8_t)(packet->size.value >> 8);
    bytes[1] = (uint8_t)(packet->size.value & 0xFF);
    crc = update_crc16(crc, bytes[0]);
    crc = update_crc16(crc, bytes[1]);

    // Process data
    for (i = 0; i < packet->size.value; i++) {
        crc = update_crc16(crc, packet->data[i]);
    }

    return crc;
}



#endif 
