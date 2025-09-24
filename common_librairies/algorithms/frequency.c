//#include "frequency.h"
//
//
//
//filter_bandpass_order1_st fbp_43hz_84hz = {
//	.b0 =  4.189885981523826e-03,
//	.b1 =  0.000000000000000,
//	.b2 = -4.189885981523826e-03,
//	.a0 =  1.000000000000000,
//	.a1 = -1.991470261811710,
//	.a2 =  0.991620228036952
//};
//
//zero_cross_freq_st zcf;
//
//// persistent variables for frequency calculation
//
//
//
//// get one voltage sample and returns filtered voltage
//#pragma CODE_SECTION(filter_bandpass, "ramfuncs");
//float filter_bandpass(const float x, filter_bandpass_order1_st* f)
//{
//	// calculate the filtered sample
//	float y = (f->b0 * x) + (f->b1 * f->x_1) + (f->b2 * f->x_2)
//		                  - (f->a1 * f->y_1) - (f->a2 * f->y_2);
//
//	// store values to next iteration
//	f->x_2 = f->x_1;
//	f->x_1 = x;
//	f->y_2 = f->y_1;
//	f->y_1 = y;
//
//	return y;
//}
//
//// return 1 if zero cross is detected and frequency calculated, return 0 otherwise
//#pragma CODE_SECTION(frequency_estimation, "ramfuncs");
//int frequency_estimation(float sample)
//{
//	static float y0 = 0;
//	static float y1 = 0;
//	static float y2 = 0;
//	static float y3 = 0;
//	static float y4 = 0;
//	static float last_time = 0;
//    static int counter = 0;
//    static int counter_positive_sample = 0;
//
//
//    float a, b;
//    float time;
//
//    counter++;
//
//    y4 = y3;
//    y3 = y2;
//    y2 = y1;
//    y1 = y0;
//	y0 = filter_bandpass(sample, &fbp_43hz_84hz);
//
//
//	switch (zcf.wave_state)
//	{
//	case PRE_TRIGGER:
//		if (y0 < 0 && y1 < 0 && y2 < 0 && y3 < 0 && y4 < 0)
//		{
//			zcf.wave_state = TRIGGER;       // if all samples are negative, ready to trigger
//			counter_positive_sample = 0;
//		}
//
//		break;
//	case TRIGGER:
//		if (y0 >= 0)
//		{
//			counter_positive_sample++;
//			if (counter_positive_sample >= 2)
//			{
//			    zcf.wave_state = POS_TRIGGER; // if two samples are positive, zero cross detected
//
//			    a = (-2 * y0 - y1 + y3 + 2 * y4) * 0.1f;   // Minimum-square linear fit
//				b = (y0 + y1 + y2 + y3 + y4) * 0.2f;
//
//				time = b / a;                              // relative time of zero cross
//
//				zcf.period = (counter + time - last_time) * SAMPLE_PERIOD; // delta time between two zero crossing
//
//				counter = 0;
//
//				zcf.frequency = 1.0f / zcf.period;
//
//				last_time = time;
//				return 1;
//			}
//		}
//		break;
//
//	case POS_TRIGGER:
//		if (y0 >= 0 && y1 >= 0 && y2 >= 0 && y3 >= 0 && y4 >= 0)
//		{
//		    zcf.wave_state = PRE_TRIGGER;  // if all samples are positive, ready to pre-trigger
//		}
//		break;
//
//	}
//
//	return 0;
//}
//
#include "frequency.h"

#ifndef CODE_SECTION
#define CODE_SECTION(fn, sect) _Pragma("CODE_SECTION(" #fn ", \"" sect "\")")
#endif

// --- Filtro passa-faixa biquad em forma direta I ---
static float filter_bandpass(float x, filter_bandpass_order1_st *f)
{
    // y[n] = (b0*x[n] + b1*x[n-1] + b2*x[n-2]) - (a1*y[n-1] + a2*y[n-2])
    float y = (f->b0 * x) + (f->b1 * f->x_1) + (f->b2 * f->x_2)
                            - (f->a1 * f->y_1) - (f->a2 * f->y_2);

    // shift de estados
    f->x_2 = f->x_1; f->x_1 = x;
    f->y_2 = f->y_1; f->y_1 = y;
    return y;
}

// --- API ---
void zc_init(zc_ctx_t *ctx, const filter_bandpass_order1_st *fcoef, float Ts)
{
    ctx->filt = *fcoef;
    // zera estados do filtro
    ctx->filt.x_1 = ctx->filt.x_2 = 0.0f;
    ctx->filt.y_1 = ctx->filt.y_2 = 0.0f;

    // zera janela das amostras filtradas
    ctx->y0 = ctx->y1 = ctx->y2 = ctx->y3 = ctx->y4 = 0.0f;

    ctx->last_time  = 0.0f;
    ctx->counter    = 0;
    ctx->counter_pos = 0;
    ctx->Ts = Ts;

    ctx->zcf.frequency = 0.0f;
    ctx->zcf.period    = 0.0f;
    ctx->zcf.state     = PRE_TRIGGER;
}

void zc_reset_runtime(zc_ctx_t *ctx)
{
    ctx->y0 = ctx->y1 = ctx->y2 = ctx->y3 = ctx->y4 = 0.0f;
    ctx->last_time  = 0.0f;
    ctx->counter    = 0;
    ctx->counter_pos = 0;
    ctx->zcf.frequency = 0.0f;
    ctx->zcf.period    = 0.0f;
    ctx->zcf.state     = PRE_TRIGGER;
}

int zc_process_sample(zc_ctx_t *ctx, float sample, float *out_freq, float *out_period)
{
    // incrementa contador de amostras desde o último zero
    ctx->counter++;

    // atualiza janela (mais antigo sai por y4)
    ctx->y4 = ctx->y3;
    ctx->y3 = ctx->y2;
    ctx->y2 = ctx->y1;
    ctx->y1 = ctx->y0;
    ctx->y0 = filter_bandpass(sample, &ctx->filt);

    switch (ctx->zcf.state)
    {
    case PRE_TRIGGER:
        if (ctx->y0 < 0 && ctx->y1 < 0 && ctx->y2 < 0 && ctx->y3 < 0 && ctx->y4 < 0) {
            ctx->zcf.state = TRIGGER;      // todos negativos: pronto para detectar cruzamento ↑
            ctx->counter_pos = 0;
        }
        break;

    case TRIGGER:
        if (ctx->y0 >= 0) {
            ctx->counter_pos++;
            if (ctx->counter_pos >= 2) {
                // confirmação de cruzamento por zero ascendente (debounce de 2 amostras)
                ctx->zcf.state = POS_TRIGGER;

                // Ajuste linear por mínimos quadrados (mesma fórmula do seu código)
                float a = (-2.0f*ctx->y0 - ctx->y1 + ctx->y3 + 2.0f*ctx->y4) * 0.1f;
                float b = ( ctx->y0 + ctx->y1 + ctx->y2 + ctx->y3 + ctx->y4) * 0.2f;

                // instante relativo do zero dentro do passo atual (em amostras)
                float t_rel = b / a;

                // período = delta_amostras * Ts
                ctx->zcf.period = ( (float)ctx->counter + t_rel - ctx->last_time ) * ctx->Ts;
                ctx->zcf.frequency = (ctx->zcf.period > 0.0f) ? (1.0f / ctx->zcf.period) : 0.0f;

                // reseta contador de amostras desde o zero
                ctx->counter = 0;
                ctx->last_time = t_rel;

                if (out_freq)   *out_freq   = ctx->zcf.frequency;
                if (out_period) *out_period = ctx->zcf.period;
                return 1;
            }
        }
        break;

    case POS_TRIGGER:
        if (ctx->y0 >= 0 && ctx->y1 >= 0 && ctx->y2 >= 0 && ctx->y3 >= 0 && ctx->y4 >= 0) {
            ctx->zcf.state = PRE_TRIGGER;  // todos positivos: volta a aguardar região negativa
        }
        break;
    }

    return 0;
}

