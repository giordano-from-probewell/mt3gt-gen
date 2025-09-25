//#############################################################################
//
//  FILE:   notch_fltr.h
//
//  TITLE:  NOTCH FLTR Module
//
//#############################################################################
// $TI Release: $
// $Release Date: $
// $Copyright: $
//#############################################################################

#ifndef NOTCH_FLTR_F_H_
#define NOTCH_FLTR_F_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Included Files
//
#include <stdint.h>
#ifndef __TMS320C28XX_CLA__
#include <math.h>
#else
#include <CLAmath.h>
#endif

//#############################################################################
//
// Macro Definitions
//
//#############################################################################
#ifndef C2000_IEEE754_TYPES
#define C2000_IEEE754_TYPES
#ifdef __TI_EABI__
typedef float         float32_t;
typedef double        float64_t;
#else // TI COFF
typedef float         float32_t;
typedef long double   float64_t;
#endif // __TI_EABI__
#endif // C2000_IEEE754_TYPES

//
// Typedefs
//
typedef struct{
	float32	B2_notch;
	float32	B1_notch;
	float32	B0_notch;
	float32	A2_notch;
	float32	A1_notch;
}NOTCH_COEFF_F;

typedef struct{
	float32	Out1;
	float32	Out2;
	float32	In;
	float32	In1;
	float32 In2;
	float32	Out;
}NOTCH_VARS_F;

//
// Function Prototypes
//
void NOTCH_FLTR_F_VARS_init(NOTCH_VARS_F *v);
void NOTCH_FLTR_F_run(NOTCH_VARS_F *v,NOTCH_COEFF_F *w);
void NOTCH_FLTR_F_ASM(NOTCH_VARS_F *v,NOTCH_COEFF_F *w);
void NOTCH_FLTR_F_COEFF_Update(float32 delta_T, float32 wn,float32 c2, float32 c1, NOTCH_COEFF_F *notch_obj);

//*********** Macro Definition ***********//
#define NOTCH_FLTR_F_MACRO(v, w)		\
	v.Out=(w.A1_notch*v.Out1)+(w.A2_notch*v.Out2)+(w.B0_notch*v.In)+(w.B1_notch*v.In1)+(w.B2_notch*v.In2); \
	v.Out2=v.Out1;	\
	v.Out1=v.Out;	\
	v.In2=v.In1;	\
	v.In1=v.In;

#ifdef __cplusplus
}
#endif // extern "C"

#endif // end of  _NOTCH_FLTR_F_H_ definition

//
// End of File
//
