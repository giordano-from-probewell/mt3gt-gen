//#############################################################################
//
//  FILE:   mppt_incc.h
//
//  TITLE:  MPPT INCC Module
//
//#############################################################################
// $TI Release: $
// $Release Date: $
// $Copyright: $
//#############################################################################

#ifndef MPPT_INCC_F_H_
#define MPPT_INCC_F_H_

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
typedef struct {
	float32_t  Ipv;
	float32_t  Vpv;
	float32_t  IpvH;
	float32_t  IpvL;
	float32_t  VpvH;
	float32_t  VpvL;
	float32_t  MaxVolt;
	float32_t  MinVolt;
	float32_t  Stepsize;
	float32_t  VmppOut;
	// internal variables
	float32_t  Cond;
	float32_t  IncCond;
	float32_t  DeltaV;
	float32_t  DeltaI;
	float32_t  VpvOld;
	float32_t  IpvOld;
	float32_t  StepFirst;
	int16_t mppt_enable;
	int16_t mppt_first;
} MPPT_INCC_F;

typedef MPPT_INCC_F *mppt_incc_handle;

//
// Function Prototypes
//
static inline void MPPT_INCC_F_init(MPPT_INCC_F *v);
static inline void MPPT_INCC_F_FUNC(MPPT_INCC_F *v);

//*********** Macro Definition ***********//
#define MPPT_INCC_F_MACRO(v)																		\
if (v.mppt_enable==1)																				\
{	/*MPPT mode enable*/																			\
	if (v.mppt_first == 1)																			\
	{																								\
		v.VmppOut= v.Vpv - v.StepFirst;																\
    	v.VpvOld=v.Vpv;																				\
    	v.IpvOld=v.Ipv;																				\
		v.mppt_first=0;																				\
	}                                                                       						\
	else 																							\
	{																								\
		v.DeltaV = v.Vpv-v.VpvOld ; 				/* PV voltage change */							\
    	v.DeltaI = v.Ipv-v.IpvOld ; 				/* PV current change */							\
   		/*if(v.DeltaV <v.VpvH && v.DeltaV >-v.VpvL )*/  /* Tolerence limits PV Voaltge Variation */	\
   		/*v.DeltaV= 0;*/																			\
   		/*if(v.DeltaI <v.IpvH && v.DeltaI >-v.IpvL )*/  /* Tolerence limits PV current Variation */	\
   		/*v.DeltaI = 0;*/																			\
    	if(v.DeltaV == 0)    						/* DeltaV=0 */									\
		{																							\
			if(v.DeltaI !=0 )    					/* dI=0 operating at MPP */						\
			{																						\
				if ( v.DeltaI > 0 )  				/* climbing towards MPP */						\
				{v.VmppOut=v.Vpv+v.Stepsize;}		/* Increment Vref */							\
				else																				\
				{v.VmppOut=v.Vpv-v.Stepsize;} 		/* Decrement Vref */							\
			}																						\
		}																							\
    	else  																						\
		{																							\
    	 	v.Cond = v.Ipv*__einvf32(v.Vpv);				/*Cond =-(v.Ipv<<10)/v.Vpv;*/			\
    	 	v.IncCond = v.DeltaI*__einvf32(v.DeltaV);		/*IncCond =(DeltaI<<10)/DeltaV;*/		\
       		if (v.IncCond != v.Cond)				/* Operating at MPP */							\
			{																						\
				if (v.IncCond > (-v.Cond))			/* left of MPP */								\
				{																					\
				v.VmppOut=v.Vpv+v.Stepsize;			/* change Vref */								\
				}																					\
				else								/* Right of MPP */								\
				{																					\
				v.VmppOut=v.Vpv-v.Stepsize;			/* change Vref */								\
				}																					\
        	}																						\
		}																							\
	    v.VmppOut=(v.VmppOut<v.MinVolt)?v.MinVolt: v.VmppOut;	 /*Min. MPP Current */  			\
		v.VmppOut=(v.VmppOut>v.MaxVolt)?v.MaxVolt: v.VmppOut;  /*Max. MPP Current	*/				\
		v.VpvOld = v.Vpv;																			\
		v.IpvOld = v.Ipv;																			\
	}																								\
}

#ifdef __cplusplus
}
#endif // extern "C"

#endif // end of  _MPPT_INCC_F_H_ definition

//
// End of File
//
