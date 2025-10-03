#ifndef ACS37800_H
#define ACS37800_H

#include <stdint.h>
#include "my_i2c.h"

// Estados principais do driver
typedef enum {
    ACS37800_NB_IDLE,
    ACS37800_NB_BUSY,
    ACS37800_NB_DONE,
    ACS37800_NB_ERROR
} acs37800_state_t;

// Tipos de operação
#define ACS_OP_NONE            0
#define ACS_OP_READ_U32        1
#define ACS_OP_WRITE_U32       2
#define ACS_OP_READ_RMS        3
#define ACS_OP_READ_POWER      4


// Estrutura principal do driver não bloqueante
typedef struct {
    i2c_context_t    *i2c_ctx;
    uint8_t           i2c_address;
    uint8_t           reg_ptr;
    uint8_t           raw[4];
    uint32_t          raw_32;
    float             result;
    float             lsb_current;
    float             lsb_voltage;
    float             lsb_power;
    float             lsb_energy;
    uint32_t             voltage;
    uint32_t             current;
    uint8_t           op_type;
    uint8_t           write_data[2];
    acs37800_state_t  state;
    int16_t           error;
} acs37800_nb_t;

// Inicialização do driver
void acs37800_nb_init(acs37800_nb_t *dev,
                      i2c_context_t *ctx,
                      uint8_t addr,
                      float lsb_current,
                      float lsb_voltage,
                      float lsb_power,
                      float lsb_energy,
                      float lsb_temp,
                      float lsb_freq);

// Comandos não bloqueantes
int acs37800_nb_start_read_reg32(acs37800_nb_t *dev, uint8_t reg);
int acs37800_nb_start_write_reg32(acs37800_nb_t *dev, uint8_t reg, uint16_t val);


int acs37800_nb_start_read_rms(acs37800_nb_t *dev);
int acs37800_nb_start_read_power(acs37800_nb_t *dev);


// Processamento e resultado
void acs37800_nb_process(acs37800_nb_t *dev);
int acs37800_nb_result(acs37800_nb_t *dev, float *out);

#endif // ACS37800_H



//// acs37800.h
//#ifndef ACS37800_H
//#define ACS37800_H
//
//#include <stdint.h>
//#include "my_i2c.h"
//
//// Estados de alto‐nível do driver
//typedef enum {
//    ACS37800_NB_IDLE,
//    ACS37800_NB_BUSY,
//    ACS37800_NB_DONE,
//    ACS37800_NB_ERROR
//} acs37800_state_t;
//
//// Fases internas
//typedef enum {
//    ACS37800_PHASE_IDLE,
//    ACS37800_PHASE_WAIT_PTR,
//    ACS37800_PHASE_WAIT_READ
//} acs37800_phase_t;
//
//// Handle não‐bloqueante
//typedef struct {
//    i2c_context_t    *i2c_ctx;     // ponteiro para seu contexto I²C
//    uint8_t           i2c_address; // endereço do chip
//    uint8_t           raw[2];      // buffer de 2 bytes
//    acs37800_phase_t  phase;       // fase interna
//    acs37800_state_t  state;       // estado externo
//    int16_t           error;       // código de erro I²C
//    int16_t           raw_counts;  // valor bruto lido
//    float             current;     // corrente calculada (A)
//    float             lsb_amp;     // sensibilidade (A/count)
//} acs37800_handle_t;
//
///**
// * @brief Inicia o driver ACS37800 (chamar apenas 1×).
// * @param h         Handle a inicializar.
// * @param i2c_ctx   Contexto I²C (my_i2c_initiate já foi chamado).
// * @param address   Endereço 7-bit do ACS37800 no barramento.
// * @param lsb_amp   Sensibilidade em A/count.
// * @return 0 em sucesso, <0 em erro.
// */
//int acs37800_nb_init(acs37800_handle_t *h,
//                     i2c_context_t    *i2c_ctx,
//                     uint8_t           address,
//                     float             lsb_amp);
//
///**
// * @brief Dispara a leitura da corrente sem bloquear.
// * Auto‐reseta o handle se não estiver IDLE.
// * @return 0 em OK, <0 em erro.
// */
//int acs37800_read_current_nb_start(acs37800_handle_t *h);
//
///**
// * @brief Avança o driver; chamar dentro do loop principal.
// */
//void acs37800_nb_process(acs37800_handle_t *h);
//
///**
// * @brief Recupera o resultado da leitura.
// * @param h            Handle após state==DONE.
// * @param out_current  Ponteiro para receber o valor em A.
// * @return 0 em sucesso, negativo em erro, -1 se ainda BUSY.
// */
//int acs37800_nb_result(acs37800_handle_t *h, float *out_current);
//
///**
// * @brief Self‐test 100% não‐bloqueante.
// * Faz write‐pointer + read e retorna a corrente em out_current.
// * @param i2c_ctx     Contexto I²C já iniciado.
// * @param address     Endereço do ACS37800.
// * @param lsb_amp     Sensibilidade em A/count.
// * @param out_current Ponteiro para float que recebe corrente (A).
// * @return 0 em sucesso, <0 em erro.
// */
//int acs37800_nb_selftest(i2c_context_t *i2c_ctx,
//                         uint8_t         address,
//                         float           lsb_amp,
//                         float          *out_current);
//
//#endif // ACS37800_H
