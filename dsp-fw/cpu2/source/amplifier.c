#include <string.h>
#include "application.h"
#include "ada4254.h"
#include "amplifier.h"
#include "amplifier_wiring.h"
#include "cli.h"

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
    mgr->app    = app;
    mgr->inputs = &app->meter.analog_input[0];

    // faça o bind lógico do ADS->AI e pré-carregue ganhos desejados/flags
    for (i = 0; i < mgr->table_len; i++) {
        const amp_wiring_t* w = &mgr->table[i];

        // analog_input_t já tem ads_channel e ada_channel
        mgr->inputs[w->ai_idx].ads_channel      = (int16_t)w->ads_ch;
        // os campos do ada_channel (id/spi/cs) serão setados ao iniciar (init_start)

        mgr->desired_in[i]  = (ada4254_in_gains_t)w->in_gain;
        mgr->desired_out[i] = (ada4254_out_gains_t)w->out_gain;
        mgr->need_gain[i]   = true;
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
            const amp_wiring_t* w = &mgr->table[mgr->current_channel];
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


//int16_t _init_analog_input(analog_input_t *input, int16_t ads_channel, uint16_t channel, uint16_t ada_spi_base, uint8_t pin_cs)
//{
//
//    ada4254_status_t status;
//
//    input->ephemeral.dc_acc = 0;
//    input->ephemeral.dc = 0;
//    input->ads_channel = ads_channel;
//    input->value = 0;
//
//    input->ada_channel.channel = channel;
//    input->ada_channel.pin_cs = pin_cs;
//
//    status = ADA4254_STATUS_NOT_INITIALIZED;
////    status = ada4254_init(&input->ada_channel, channel, ada_spi_base ,pin_cs);
////    if (status == ADA4254_STATUS_OK)
////    {
////        status = ada4254_set_gain(&input->ada_channel, IN_GAIN_1, OUT_GAIN_1);
////        status = ada4254_verify(&input->ada_channel);
////    }
//    input->ada_channel.status = status;
//
//    return status;
//}


//int16_t _amplifier_init(void)
//{
//    cli_update_display_raw("Amplifier IN: ");
//   status = _init_analog_input(&app->meter.analog_input[0], ADS178_CH1, ADA4254_A8, SPIA_BASE, GPIO_PIN_CS_A8 );// IN
//   ada4254_set_gain(&app->meter.analog_input[0].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//
//   cli_update_display_raw("Amplifier VN: ");
//   status = _init_analog_input(&app->meter.analog_input[1], ADS178_CH2, ADA4254_A4, SPIA_BASE, GPIO_PIN_CS_A4 );// VN
//   ada4254_set_gain(&app->meter.analog_input[1].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//
//
//   cli_update_display_raw("Amplifier IC: ");
//   status = _init_analog_input(&app->meter.analog_input[2], ADS178_CH3, ADA4254_A7, SPIA_BASE, GPIO_PIN_CS_A7 );// IC
//   ada4254_set_gain(&app->meter.analog_input[2].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//   cli_update_display_raw("Amplifier VC: ");
//   status = _init_analog_input(&app->meter.analog_input[3], ADS178_CH4, ADA4254_A3, SPIA_BASE, GPIO_PIN_CS_A3 );// VC
//   ada4254_set_gain(&app->meter.analog_input[3].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//
//
//
//   cli_update_display_raw("Amplifier IB: ");
//   status = _init_analog_input(&app->meter.analog_input[4], ADS178_CH5, ADA4254_A6, SPIA_BASE, GPIO_PIN_CS_A6 );// IB
//   ada4254_set_gain(&app->meter.analog_input[4].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//   cli_update_display_raw("Amplifier VB: ");
//   status = _init_analog_input(&app->meter.analog_input[5], ADS178_CH6, ADA4254_A2, SPIA_BASE, GPIO_PIN_CS_A2 );// VB
//   ada4254_set_gain(&app->meter.analog_input[5].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//
//
//
//   cli_update_display_raw("Amplifier IA: ");
//   status = _init_analog_input(&app->meter.analog_input[6], ADS178_CH7, ADA4254_A5, SPIA_BASE, GPIO_PIN_CS_A5 );// IA
//   ada4254_set_gain(&app->meter.analog_input[6].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//   cli_update_display_raw("Amplifier VA: ");
//   status = _init_analog_input(&app->meter.analog_input[7], ADS178_CH8, ADA4254_A1, SPIA_BASE, GPIO_PIN_CS_A1 );// VA
//   ada4254_set_gain(&app->meter.analog_input[7].ada_channel, IN_GAIN_1 , OUT_GAIN_1 );
//   if(status <= 0 )
//       cli_update_display_raw("Nok\n\r");
//   else
//       cli_update_display_raw("Ok\n\r");
//
//}

//void _amplifier_new_gain(void)
//{
//    if (app.meter.analog_input[0].config.flag_new_range)
//    {
//        app.meter.analog_input[0].config.flag_new_range = false;
//        // change ada gain 1 or 8
//        // app.meter.analog_input[0].config.current_range = 0 or 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 8
//        cla_phase_aux.calibration_index = app.meter.analog_input[0].config.current_range;
//
//        if (cla_phase_aux.calibration_index == 0)
//            ada4254_set_gain(&app.meter.analog_input[0].ada_channel, IN_GAIN_8, OUT_GAIN_1);
//        if (cla_phase_aux.calibration_index == 1)
//            ada4254_set_gain(&app.meter.analog_input[0].ada_channel, IN_GAIN_1, OUT_GAIN_1);
//        if (cla_phase_aux.calibration_index == 2)
//            ada4254_set_gain(&app.meter.analog_input[0].ada_channel, IN_GAIN_0_125, OUT_GAIN_1);
//    }
//    if (app.meter.analog_input[2].config.flag_new_range)
//    {
//        app.meter.analog_input[2].config.flag_new_range = false;
//        // change ada gain 1 or 8
//        // app.meter.analog_input[0].config.current_range = 0 or 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 8
//        cla_phase_c.calibration_index = app.meter.analog_input[2].config.current_range;
//
//        if (cla_phase_c.calibration_index == 0)
//            ada4254_set_gain(&app.meter.analog_input[2].ada_channel, IN_GAIN_8, OUT_GAIN_1);
//        if (cla_phase_c.calibration_index == 1)
//            ada4254_set_gain(&app.meter.analog_input[2].ada_channel, IN_GAIN_1, OUT_GAIN_1);
//        if (cla_phase_c.calibration_index == 2)
//            ada4254_set_gain(&app.meter.analog_input[2].ada_channel, IN_GAIN_0_125, OUT_GAIN_1);
//    }
//    if (app.meter.analog_input[4].config.flag_new_range)
//    {
//        app.meter.analog_input[4].config.flag_new_range = false;
//        // change ada gain 1 or 8
//        // app.meter.analog_input[0].config.current_range = 0 or 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 8
//        cla_phase_b.calibration_index = app.meter.analog_input[4].config.current_range;
//
//        if (cla_phase_b.calibration_index == 0)
//            ada4254_set_gain(&app.meter.analog_input[4].ada_channel, IN_GAIN_8, OUT_GAIN_1);
//        if (cla_phase_b.calibration_index == 1)
//            ada4254_set_gain(&app.meter.analog_input[4].ada_channel, IN_GAIN_1, OUT_GAIN_1);
//        if (cla_phase_b.calibration_index == 2)
//            ada4254_set_gain(&app.meter.analog_input[4].ada_channel, IN_GAIN_0_125, OUT_GAIN_1);
//    }
//    if (app.meter.analog_input[6].config.flag_new_range)
//    {
//        app.meter.analog_input[6].config.flag_new_range = false;
//        // change ada gain 1 or 8
//        // app.meter.analog_input[0].config.current_range = 0 or 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 1
//        // app.meter.analog_input[0].config.current_range = 0 -> ada gain = 8
//        cla_phase_a.calibration_index = app.meter.analog_input[6].config.current_range;
//
//        if (cla_phase_a.calibration_index == 0)
//            ada4254_set_gain(&app.meter.analog_input[6].ada_channel, IN_GAIN_8, OUT_GAIN_1);
//        if (cla_phase_a.calibration_index == 1)
//            ada4254_set_gain(&app.meter.analog_input[6].ada_channel, IN_GAIN_1, OUT_GAIN_1);
//        if (cla_phase_a.calibration_index == 2)
//            ada4254_set_gain(&app.meter.analog_input[6].ada_channel, IN_GAIN_0_125, OUT_GAIN_1);
//    }
//
//}




