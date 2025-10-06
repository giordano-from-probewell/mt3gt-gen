#include "generation_sm.h"
#include <math.h>

// helpers
static inline float32_t _step_towards(float32_t cur, float32_t tgt, float32_t step)
{
    if (tgt > cur) { float32_t nx = cur + step; return (nx > tgt) ? tgt : nx; }
    if (tgt < cur) { float32_t nx = cur - step; return (nx < tgt) ? tgt : nx; }
    return cur;
}

void gen_sm_init(gen_runtime_t* r, gen_profile_t prof)
{
    (void)prof;
    *r = (gen_runtime_t){0};
    r->st = GEN_ST_IDLE;
}

void gen_sm_set_target(gen_runtime_t* r, gen_target_t t)
{
    r->tgt = t;
    // if waveform changed
    r->pending_wave_switch = (t.waveform_target != r->cur.waveform_target);
    // if gen was in idle, ramp up
    if (r->st == GEN_ST_IDLE || r->st == GEN_ST_HOLD) {
        r->st = r->pending_wave_switch ? GEN_ST_RAMP_DOWN : GEN_ST_RAMP_UP;
    }
}

void gen_sm_turning_on(gen_runtime_t* r){

    //Bridge V En - on
    GPIO_writePin(BRIDGE_V_EN_PIN, 1);
    //Bridge I En - on
    GPIO_writePin(BRIDGE_I_EN_PIN, 1);

    GPIO_writePin(SDFM_CLK_SEL_PIN,0);                              // SDFM_CLK_SEL: 0 from ext STD; 1 from int SI5351
    GPIO_writePin(CLK_AUX_OE_PIN,1);                                // CLK_AUX_OE = 1 clk out enabled

    link_SDFM_to_CLA();

    GPIO_writePin(FB_EN_PIN, 1);
}


void gen_sm_on_zero_cross(gen_runtime_t* r) {
    r->zc_tick = true;
}

void gen_sm_on_protection(gen_runtime_t* r) {
    r->st = GEN_ST_FAULT;
}

// Aplicar setpoints (CPU1 → CLA): use seus campos/setpoints atuais
static void _apply_setpoints(application_t* app,
                             float32_t v_scale, float32_t i_scale,
                             uint8_t waveform) {
    // Exemplo: atualize os setpoints usados pela CLA
    // Você já deixa os controladores na CLA e atualiza setpoints externamente. :contentReference[oaicite:2]{index=2}
    app->generation.voltage.config.scale  = v_scale;
    app->generation.current.config.scale  = i_scale;
//    app->generation.voltage.ref.waveform  = waveform;
//    app->generation.current.ref.waveform  = waveform;
}

bool gen_sm_process(gen_runtime_t* r,
                     gen_profile_t prof,
                     application_t* app)
{
    if (!r->zc_tick) return false;     // só avança por zero-cross
    r->zc_tick = false;

    bool changed = false;

    switch (r->st) {
    case GEN_ST_IDLE:
        // nada a fazer
        break;

    case GEN_ST_RAMP_DOWN: {
        float32_t step = fmaxf(prof.slew_down_per_zc, prof.min_step);
        float32_t v = _step_towards(r->cur.scale_target, 0.0f, step);
        r->cur.scale_target = v;
        _apply_setpoints(app, v, v, r->cur.waveform_target);
        changed = true;

        if (v <= 0.0f + 1e-6f) {
            r->st = r->pending_wave_switch ? GEN_ST_SWITCH_WAVEFORM : GEN_ST_RAMP_UP;
        }
    } break;

    case GEN_ST_SWITCH_WAVEFORM:
        r->cur.waveform_target = r->tgt.waveform_target; // troca “em zero”
        r->pending_wave_switch = false;
        r->st = GEN_ST_RAMP_UP;
        // aplica já com 0 de escala (continua em zero)
        _apply_setpoints(app, 0.0f, 0.0f, r->cur.waveform_target);
        changed = true;
        break;

    case GEN_ST_RAMP_UP: {
        float32_t step = fmaxf(prof.slew_up_per_zc, prof.min_step);
        float32_t v = _step_towards(r->cur.scale_target, r->tgt.scale_target, step);
        r->cur.scale_target = v;
        _apply_setpoints(app, v, v, r->cur.waveform_target);
        changed = true;

        if (fabsf(v - r->tgt.scale_target) <= 1e-6f) {
            r->st = GEN_ST_HOLD;
        }
    } break;

    case GEN_ST_HOLD:
        // alvos podem mudar a qualquer momento via gen_fsm_set_target()
        break;

    case GEN_ST_FAULT:
        // mantenha em zero e aguarde reset externo (proteção/CLI)
        _apply_setpoints(app, 0.0f, 0.0f, r->cur.waveform_target);
        changed = true;
        break;
    }

    return changed;
}
