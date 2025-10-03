// tmp75.h
#ifndef TMP75_H
#define TMP75_H

#include <stdint.h>
#include "my_i2c.h"

#define TMP75_DEFAULT_ADDRESS 0x48

// Estados de alto-nível do driver
typedef enum {
    TMP75_NB_IDLE,
    TMP75_NB_BUSY,
    TMP75_NB_DONE,
    TMP75_NB_ERROR
} tmp75_nb_state_t;


typedef struct {
    volatile struct I2C_REGS *i2c_regs;
    uint8_t  i2c_address;
    float32_t temperature;
    uint8_t cfg;
    uint8_t  raw[2];       // buffer bruto de 2 bytes
    uint16_t error_counter;
} tmp75_info_t;


// Handle para leitura não-bloqueante do TMP75
typedef struct {
    tmp75_info_t   *info;
    i2c_context_t   *i2c_ctx;      // contexto I²C compartilhado
    tmp75_nb_state_t state;
} tmp75_nb_handle_t;

int tmp75_nb_init(tmp75_nb_handle_t *h, tmp75_info_t *info, i2c_context_t *i2c_ctx);

/**
 * Dispara a leitura de temperatura não-bloqueante.
 * @return 0 em OK, <0 em erro.
 */
int tmp75_read_temp_nb_start(tmp75_nb_handle_t *h);

/**
 * Deve ser chamado repetidamente no loop principal para avançar o driver.
 */
void tmp75_nb_process(tmp75_nb_handle_t *h);

/**
 * Recupera o resultado:
 *   0 se OK (e preenche *out_temp),
 *  <0 se erro,
 *  -1 se ainda está ocupado.
 */
int tmp75_nb_result(tmp75_nb_handle_t *h, float32_t *out_temp);

int tmp75_nb_selftest(tmp75_info_t *info, i2c_context_t *i2c_ctx);

int tmp75_selftest(tmp75_info_t *info, i2c_context_t *i2c_ctx);

#endif // TMP75_H

