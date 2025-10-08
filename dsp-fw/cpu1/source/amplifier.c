#include <string.h>
#include "application.h"
#include "ada4254.h"
#include "amplifier.h"
#include "amplifier_wiring.h"

// manager singleton
static amplifier_manager_t s_mgr;

static uint8_t _mgr_slot_from_ai_idx(const amplifier_manager_t* mgr, uint8_t ai_idx) {
    uint8_t i;
    for (i = 0; i < mgr->table_len; i++) {
        if (mgr->table[i].ai_idx == ai_idx) return i;
    }
    return 0xFFu;
}

void amplifier_mgr_init(amplifier_manager_t* mgr)
{
    memset(mgr, 0, sizeof(*mgr));
    mgr->state = AMP_MGR_IDLE;
    mgr->table = AMP_WIRING;
    mgr->table_len = AMP_WIRING_N; // da tabela
}

void amplifier_mgr_bind_from_table(amplifier_manager_t* mgr, application_t * app)
{
    uint8_t i;
    mgr->inputs = &app->measures.analog_input[0];


    for (i = 0; i < mgr->table_len; i++) {
         amp_wiring_t* w = &mgr->table[i];

        mgr->desired_in[i]  = (ada4254_in_gains_t)w->in_gain;
        mgr->desired_out[i] = (ada4254_out_gains_t)w->out_gain;
        mgr->need_gain[i]   = true;
        if(w->ai_idx == AFE_VV1)
            app->measures.primary.voltage.voltage_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_VI1)
            app->measures.primary.voltage.current_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_IV1)
            app->measures.primary.current.voltage_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_II1)
            app->measures.primary.current.current_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_VV2)
            app->measures.secondary.voltage.voltage_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_VI2)
            app->measures.secondary.voltage.current_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_IV2)
            app->measures.secondary.current.voltage_gain = &mgr->inputs[i].ada_channel;
        if(w->ai_idx == AFE_II2)
            app->measures.secondary.current.current_gain = &mgr->inputs[i].ada_channel;

    }



}

void amplifier_mgr_start(amplifier_manager_t* mgr)
{
    mgr->current_channel = 0;
    mgr->state = AMP_MGR_INIT_NEXT;
}

void amplifier_mgr_poll(amplifier_manager_t* mgr, my_time_t now)
{
    switch (mgr->state)
    {
    case AMP_MGR_IDLE:
        break;

    case AMP_MGR_INIT_NEXT:
        if (mgr->current_channel >= mgr->table_len) {
            mgr->current_channel = 0;
            mgr->state = AMP_MGR_SETGAIN_NEXT;
            break;
        } else {
            amp_wiring_t* w = &mgr->table[mgr->current_channel];
            analog_input_t* ai = &mgr->inputs[w->ai_idx];

            // inicia init não-blocante deste ADA (ctx único reaproveitado por canal)
            ada4254_init_start(&mgr->init_ctx,
                               &ai->ada_channel,
                               w->ada_id,
                               w->spi_base,
                               w->cs_pin);
            mgr->state = AMP_MGR_INIT_POLL;
        }
        break;

    case AMP_MGR_INIT_POLL: {
        ada4254_status_t st = ada4254_init_poll(&mgr->init_ctx, now);
        if (st != ADA4254_STATUS_NOT_INITIALIZED) {
            // terminou (OK ou erro). Avança para o próximo canal
            mgr->current_channel++;
            mgr->state = AMP_MGR_INIT_NEXT;
        }
        break;
    }

    case AMP_MGR_SETGAIN_NEXT:
        if (mgr->current_channel >= mgr->table_len) {
            mgr->state = AMP_MGR_INITIATED;
            break;
        } else {
            const amp_wiring_t* w = &mgr->table[mgr->current_channel];
            analog_input_t* ai = &mgr->inputs[w->ai_idx];

            if (mgr->need_gain[mgr->current_channel]) {
                // inicia set gain não-blocante
                ada4254_set_gain_start(&mgr->gain_ctx,
                                       &ai->ada_channel,
                                       mgr->desired_in[mgr->current_channel],
                                       mgr->desired_out[mgr->current_channel]);
                mgr->state = AMP_MGR_SETGAIN_POLL;
            } else {
                mgr->current_channel++;
                // fica no mesmo estado para verificar próximo
            }
        }
        break;

    case AMP_MGR_SETGAIN_POLL: {
        ada4254_status_t st = ada4254_set_gain_poll(&mgr->gain_ctx, now);
        if (st != ADA4254_STATUS_NOT_INITIALIZED) {
            // end (OK r NOK). Go to next
//            cli_update_display_raw((char*)mgr->table[mgr->current_channel].label);
//            if(st == ADA4254_STATUS_OK)
//                cli_update_display_raw(" Ok\n\r");
//            else
//                cli_update_display_raw(" Nok\n\r");
            mgr->need_gain[mgr->current_channel] = false;
            mgr->current_channel++;
            mgr->state = AMP_MGR_SETGAIN_NEXT;
        }
        break;
    }
    case AMP_MGR_INITIATED:
        //cli_update_display_raw("Amplifiers ready!\n\r");
        mgr->state = AMP_MGR_DONE;
        break;

    case AMP_MGR_DONE:
        // nada a fazer
        break;
    }

}


// System API wrappers
void amplifier_system_start(application_t* app)
{
    amplifier_mgr_init(&s_mgr);
    amplifier_mgr_bind_from_table(&s_mgr, app);
    amplifier_mgr_start(&s_mgr);
}

void amplifier_system_poll(my_time_t now)
{
    amplifier_mgr_poll(&s_mgr, now);
}

bool amplifier_system_ready(void)
{
    return !amplifier_is_busy();
}


bool amplifier_request_gain(amp_channel_t afe_ch,
                            ada4254_in_gains_t in_gain,
                            ada4254_out_gains_t out_gain)
{
    amplifier_manager_t* mgr = &s_mgr;
    s_mgr.current_channel = 0;
    s_mgr.state = AMP_MGR_SETGAIN_NEXT;

    uint8_t slot = _mgr_slot_from_ai_idx(mgr, afe_ch);
    if (slot == 0xFFu) return false;

    mgr->desired_in[slot]  = in_gain;
    mgr->desired_out[slot] = out_gain;
    mgr->need_gain[slot]   = true;     // o scanner aplicará assim que alcançar o slot
    return true;
}


bool amplifier_is_busy(void)
{
    uint8_t i;
    amplifier_manager_t* mgr = &s_mgr;
    // ocupado se está no meio de um set_gain ou se há pedidos pendentes
    if (mgr->state == AMP_MGR_SETGAIN_POLL) return true;
    for (i = 0; i < mgr->table_len; i++) {
        if (mgr->need_gain[i]) return true;
    }
    return false;
}

