#ifndef CRC8_H_
#define CRC8_H_


#include <stdio.h>

#define		CRC_START_8		0x00

uint8_t CRC_8(uint8_t *input_str, size_t num_bytes );
uint8_t update_crc_8(uint8_t crc, uint8_t val );


#endif /* CRC8_H_ */
