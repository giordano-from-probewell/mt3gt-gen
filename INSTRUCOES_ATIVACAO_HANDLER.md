# Como Ativar o Handler IPC para Comando set_scale_voltage

## Resumo do Fluxo de Comunicação

```
CPU2 (CLI) ──► IPC Command ──► CPU1 (Handler) ──► Máquina de Estados
```

## Passo a Passo para Ativação

### 1. Ativar o Handler na CPU1

No arquivo `dsp-fw/application/ipc_handlers.c`, encontre a função `handle_generation_command_cpu1()` e localize o caso:

```c
case IPC_CMD_GEN_SET_SCALE:
{
    uint8_t channel = payload[0];
    float scale_voltage = 0.0f;
    memcpy(&scale_voltage, &payload[1], sizeof(float));
    
    if (channel == 0) {
        // DESCOMENTE A LINHA ABAIXO:
        // gen_sm_request_scale(&app.generation.voltage, scale_voltage);
        ↑ REMOVER OS "//" DESTA LINHA
    }
    break;
}
```

**AÇÃO NECESSÁRIA**: Remover o comentário `//` da linha para ativar:
```c
gen_sm_request_scale(&app.generation.voltage, scale_voltage);
```

### 2. Estrutura da Aplicação

A aplicação global `app` contém os canais de geração:

```c
extern application_t app;  // Definido em application.c

// Canais disponíveis:
app.generation.voltage  // Canal de tensão
app.generation.current  // Canal de corrente
```

### 3. Estados da Máquina de Geração

A função `gen_sm_request_scale()` controla os seguintes estados:

- `GEN_SM_OFF`: Ponte desligada, escala = 0
- `GEN_SM_ARMING`: Armando, preparando para ligar  
- `GEN_SM_RAMP_UP`: Subindo escala atual → escala solicitada
- `GEN_SM_RUN`: Funcionando na escala solicitada
- `GEN_SM_FAULT`: Falha, requer reset externo

### 4. Comandos IPC Disponíveis

Para enviar da CPU2 para CPU1:

```c
// Método recomendado (macro):
IPC_SEND_GEN_SET_SCALE(0, scale_value);  // 0 = voltage channel

// Método manual:
ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);

// Método baixo nível:
ipc_send_raw_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
```

### 5. Integração com CLI (CPU2)

Exemplo de função CLI:

```c
void cli_cmd_voltage_scale(float scale) {
    if (scale < 0.0f || scale > 10.0f) {
        CLI_LOGE("Invalid scale: %.2f", scale);
        return;
    }
    
    // Enviar para CPU1
    IPC_SEND_GEN_SET_SCALE(0, scale);
    
    CLI_LOGI("Voltage scale %.2f sent to CPU1", scale);
}
```

### 6. Verificação de Funcionamento

Para verificar se está funcionando:

1. **CPU2**: Adicione logs no envio do comando IPC
2. **CPU1**: Adicione logs no handler `handle_generation_command_cpu1()`
3. **State Machine**: Adicione logs na função `gen_sm_request_scale()`

### 7. Debug e Troubleshooting

- **Verificar IPC**: Use `ipc_get_status()` para verificar comunicação
- **Verificar Handler**: Adicione breakpoints em `handle_generation_command_cpu1()`
- **Verificar State Machine**: Monitore `app.generation.voltage.state`

## Arquivos Modificados

1. `dsp-fw/application/ipc_handlers.c` - Descomentar linha do handler
2. `dsp-fw/cpu2/source/app_cpu2.c` - Adicionar comando CLI (opcional)
3. Usar exemplos em `exemplo_cmd_set_scale_voltage.c`

## Comando para Testar

Depois de ativar o handler, teste no CLI da CPU2:

```
> voltage_scale 5.0
```

Isso deve:
1. Enviar comando IPC da CPU2 para CPU1
2. CPU1 receber e processar o comando
3. Máquina de estados de geração ajustar a escala de tensão