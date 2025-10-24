# Correção do Problema IPC: cmd_set_scale_voltage com len ≠ 1

## 🚨 Problema Identificado

O comando `cmd_set_scale_voltage` estava falhando no `ipc_dispatch_command` porque:

### ❌ **Situação Anterior:**
```c
// Registro do handler com expected_len = 1
ipc_register_handler(IPC_CMD_GEN_SET_SCALE, "GEN_CMD", 1, handle_generation_command_cpu1);

// Mas o macro envia 5 bytes:
#define IPC_SEND_GEN_SET_SCALE(ch, scale_val) do { \
    uint8_t payload[5]; \
    payload[0] = (ch); \
    memcpy(&payload[1], &(scale_val), sizeof(float)); \
    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5); \
} while(0)
```

### 🔍 **Root Cause:**
```c
// Em ipc_dispatch_command (ipc_comm.c):
if (s_command_handlers[i].expected_len != 0 && 
    s_command_handlers[i].expected_len != len) {
    s_ipc_state.stats.handler_errors++;
    return;  // ← COMANDO ERA REJEITADO AQUI!
}
```

**Conflito**: Handler esperava 1 byte, mas comando enviava 5 bytes.

## ✅ **Solução Implementada**

### 1. **Correção do Registro do Handler**
```c
// ANTES:
ipc_register_handler(IPC_CMD_GEN_SET_SCALE, "GEN_CMD", 1, handle_generation_command_cpu1);

// DEPOIS:
ipc_register_handler(IPC_CMD_GEN_SET_SCALE, "GEN_CMD", 5, handle_generation_command_cpu1);
//                                                       ↑ 
//                                            1 byte channel + 4 bytes float
```

### 2. **Atualização do Handler**
```c
// ANTES: Tratava payload[0] como sub-comando
uint8_t cmd = payload[0];
switch (cmd) {
    case 3: // Set scale
        if (len >= 5) {
            float scale;
            memcpy(&scale, &payload[1], sizeof(float));
            gen_sm_request_scale(&app.generation.voltage, scale);
        }
        break;
}

// DEPOIS: Trata payload[0] como canal diretamente
if (len == 5) {  // SET_SCALE command: channel + float
    uint8_t channel = payload[0];
    float scale;
    memcpy(&scale, &payload[1], sizeof(float));
    
    if (channel == 0) {
        gen_sm_request_scale(&app.generation.voltage, scale);
    } else if (channel == 1) {
        gen_sm_request_scale(&app.generation.current, scale);
    }
    return true;
}
```

## 📋 **Arquivos Modificados**

1. **`dsp-fw/application/ipc_handlers.c`**:
   - Registro do handler: `expected_len = 1` → `expected_len = 5`
   - Handler atualizado para processar canal e valor corretamente

## 🧪 **Como Testar**

### Via CLI Serial (CPU2):
```
> cmd_set_scale_voltage 5.0
```

### Via Função Direta (CPU2):
```c
cmd_set_scale_voltage_cpu2(5.0f);
```

### Resultado Esperado:
```
[CPU2] Voltage scale 5.00 sent to CPU1
[CPU1] Scale voltage set to 5.00 on channel 0
```

## 🔍 **Debugging**

Para verificar se está funcionando:

1. **Monitorar estatísticas IPC**:
   ```c
   // Verificar se s_ipc_state.stats.handler_errors não aumenta
   ```

2. **Adicionar logs no handler**:
   ```c
   if (len == 5) {
       CLI_LOGI("Received SET_SCALE: ch=%d, scale=%.2f", channel, scale);
   }
   ```

3. **Verificar estado da máquina**:
   ```c
   // Monitorar app.generation.voltage.state
   ```

## 📝 **Lição Aprendida**

Sempre verificar que o `expected_len` no registro do handler corresponde exatamente ao tamanho do payload enviado. O sistema IPC valida rigorosamente o comprimento dos comandos para garantir integridade dos dados.

### Estrutura do Comando SET_SCALE:
```
Byte 0: Channel (0=voltage, 1=current)
Byte 1-4: Float value (scale)
Total: 5 bytes
```