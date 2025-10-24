/**
 * @file exemplo_cmd_set_scale_voltage.c
 * @brief Exemplo de como enviar comando set_scale_voltage da CPU2 para CPU1
 */

#include "ipc_handlers.h"

// ============================================================================
// MÉTODO 1: USANDO MACRO SIMPLIFICADO (RECOMENDADO)
// ============================================================================

void cmd_set_scale_voltage_cpu2(float scale_value) {
    // Canal 0 = Voltage, Canal 1 = Current
    uint8_t channel = 0;  // Voltage channel
    
    // Enviar comando usando macro
    IPC_SEND_GEN_SET_SCALE(channel, scale_value);
    
    // Log para debug
    CLI_LOGI("Scale voltage set to %.2f sent to CPU1", scale_value);
}

// ============================================================================
// MÉTODO 2: USANDO FUNÇÃO DIRETA COM PAYLOAD CUSTOMIZADO
// ============================================================================

void cmd_set_scale_voltage_manual(float scale_value) {
    // Preparar payload manualmente
    uint8_t payload[5];
    payload[0] = 0;  // Canal 0 = Voltage
    memcpy(&payload[1], &scale_value, sizeof(float));  // Float nos próximos 4 bytes
    
    // Enviar usando função direta
    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
    
    CLI_LOGI("Manual scale voltage %.2f sent to CPU1", scale_value);
}

// ============================================================================
// MÉTODO 3: USANDO FUNÇÃO DE BAIXO NÍVEL
// ============================================================================

void cmd_set_scale_voltage_lowlevel(float scale_value) {
    uint8_t payload[5];
    payload[0] = 0;  // Canal 0 = Voltage
    memcpy(&payload[1], &scale_value, sizeof(float));
    
    // Enviar usando função de baixo nível do IPC
    ipc_send_raw_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
    
    CLI_LOGI("Low-level scale voltage %.2f sent to CPU1", scale_value);
}

// ============================================================================
// EXEMPLO DE USO NO CLI DA CPU2
// ============================================================================

void cli_cmd_voltage_scale(float scale) {
    // Validar entrada
    if (scale < 0.0f || scale > 10.0f) {
        CLI_LOGE("Invalid scale value: %.2f (must be 0.0-10.0)", scale);
        return;
    }
    
    // Enviar comando para CPU1
    cmd_set_scale_voltage_cpu2(scale);
    
    // Opcional: Atualizar variável local para tracking
    cli_new_scale = scale;
}

// ============================================================================
// EXEMPLO DE INTEGRAÇÃO COM SISTEMA EXISTENTE
// ============================================================================

void update_voltage_scale_from_cli(void) {
    static float last_voltage_scale = -1.0f;
    
    // Verificar se houve mudança na escala de tensão via CLI
    if (cli_new_scale != last_voltage_scale) {
        
        // Enviar comando para CPU1
        cmd_set_scale_voltage_cpu2(cli_new_scale);
        
        // Atualizar tracking
        last_voltage_scale = cli_new_scale;
        
        CLI_LOGI("Voltage scale updated to %.2f", cli_new_scale);
    }
}