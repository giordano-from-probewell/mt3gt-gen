; acos_f32.asm - assembly implementation of acosf for FPU32 with TMU
;
; Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
; ALL RIGHTS RESERVED 

;-----------------------------------------------------------------------------

; C prototype: extern float acosf(float theta)
; argument 1 = theta : [R0H] input [-1 : +1]
; return = [R0H] angle in radians [-pi : +pi]

		.if __TI_EABI__
		.asg	acosf, _acos
		.endif
		.global _acos

	   .if (__TI_EABI__ = 0)
       ; For TI COFF double and float are the same size, so atanf() and atan()
       ; should be the same function
       .global _acosf

       .align	2
_acosf:
	   .endif
_acos:
		.asmfunc
		.if (.TMS320C2800_TMU0 = 1)
		MOV32   		*SP++, R7H				; push R7H
		ABSF32			R1H, R0H				; phi = |theta|
		MPYF32			R2H, R0H, R0H			; R2H = phi^2
		MOVIZF32		R7H, #1.0				; R7H = 1.0f
		SUBF32			R2H, R7H, R2H			; R2H = 1 - phi^2
		NOP
		CMPF32			R1H, R7H				; test |theta| > 1
		MOVST0			NF, ZF					; copy flags
		SB				acos_nan, GT			; NaN
		SQRTF32			R2H, R2H				; R2H = sqrt(1 = phi^2)
		CMPF32			R0H, #0.0				; test sgn(theta)
		MOV32			*SP++, STF				; push flags
		NOP
		NOP
		DIVF32			R2H, R2H, R1H			; R2H = acos_v = sqrt(1 = phi^2) / phi
		NOP
		NOP
		NOP
		NOP
		DIVF32			R3H, R7H, R2H			; R3H = 1 / acos_v
		MOVF32			R1H, #1.570796327		; R1H = pi / 2
		NOP
		NOP
		ATANPUF32		R0H, R2H				; R0H = atan(acos_v)
		ATANPUF32		R3H, R3H				; R3H = atan(1 / acos_v)
		NOP
		NOP
		MPY2PIF32		R0H, R0H				; R0H = 2*pi*atan(acos_v)
		MPY2PIF32		R3H, R3H				; R3H = 2*pi*atan(1 / acos_v)
		NOP
		NOP
		SUBF32			R3H, R1H, R3H			; R3H = (pi/2) - 2*pi*atan(1 / acos_v)
		CMPF32			R2H, R7H				; set flags on acos_v
		MOV32			R0H, R3H, GT			; select accos_b
		MOV32			STF, *--SP				; pop flags
		MOVST0			NF						; copy NF flag
		SB				acos_end, GEQ			; branch if (theta >= 0)
		MOVF32			R1H, 3.141592654		; R1H = pi
		SUBF32			R0H, R1H, R0H			; R0H = pi - acos_b
acos_end:
		MOV32   		R7H, *--SP, UNC			; pop R7H
		LRETR
acos_nan:
		MOVIZ			R0H, #0x7FC0			; R0H = NaN
		MOV32   		R7H, *--SP, UNC			; pop R7H
		LRETR
		.else
		.wmsg			"WARNING - TMU0 NOT PRESENT: _acosf NOT BUILT"
		LRETR
		.endif
		.endasmfunc

;-----------------------------------------------------------------------------

		.end

; end of file
