/* stc8h.h - Direcciones SFR verificadas STC8H8K64U (Ref.Manual 2022/3/9) */
#ifndef STC8H_H
#define STC8H_H

#include <stdint.h>

/* --- Puerto 2 (5 LEDs PWM: P2.0-P2.4) --- */
__sfr __at(0xA0) P2;       /* Datos puerto 2 */
__sfr __at(0x95) P2M1;     /* Modo bit alto P2 */
__sfr __at(0x96) P2M0;     /* Modo bit bajo P2 */

/* --- Timer0 --- */
__sfr __at(0x8A) TL0;      /* Contador bajo / Recarga bajo */
__sfr __at(0x8C) TH0;      /* Contador alto / Recarga alto */
__sfr __at(0x88) TCON;     /* TF0(bit5), TR0(bit4) */
__sfr __at(0x89) TMOD;     /* Bits [3:0]: GATE/C/T/M1/M0 Timer0 */
__sfr __at(0x8E) AUXR;     /* T0x12(bit7): 1=16bit auto-reload, 0=modo clásico */

/* --- Interrupciones --- */
__sfr __at(0xA8) IE;       /* EA(bit7), ET0(bit1) */

/* --- Bits SFR direccionables --- */
__sbit __at(0x8C) TR0;     /* TCON.4: Timer0 Run Control */
__sbit __at(0xA9) ET0;     /* IE.1: Timer0 Interrupt Enable */
__sbit __at(0xAF) EA;      /* IE.7: Global Interrupt Enable */

#endif /* STC8H_H */