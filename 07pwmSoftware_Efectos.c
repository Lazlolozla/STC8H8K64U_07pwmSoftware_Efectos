/* 07pwmSoftware_Efectos.c - 3 efectos para 5 LEDs con PWM software (STC8H @ 24MHz) */
#include "stc8h.h"

/* Array de brillo para 5 LEDs: índice 0=P2.0, 1=P2.1, ..., 4=P2.4 */
/* volatile porque ISR lee y superloop escribe */
volatile uint8_t brillo[5] = {0, 0, 0, 0, 0};

/* Contador de ciclo PWM: 0-255, overflow natural de uint8_t */
static volatile uint8_t pwm_ciclo = 0;

/* Base de tiempo derivada del PWM: cuenta overflows (~1ms cada uno) */
static volatile uint8_t tick_ms = 0;

/* Flag para indicar actualización de efecto en superloop */
volatile uint8_t flag_efecto = 0;

/**
 * @brief ISR Timer0 - Vector 0x000B (interrupción #1)
 * Ejecutada cada 4µs. Genera PWM para 5 LEDs por comparación directa.
 * SIN BUCLES: 5 comparaciones desrolladas para minimizar latencia.
 * También genera base de tiempo de ~20ms para actualizar efectos.
 */
void isr_timer0(void) __interrupt(1) {
    /* === COMPARACIÓN PWM LED0 (P2.0) === */
    /* Si ciclo actual < brillo deseado → pin LOW (NMOS ON, LED enciende) */
    if (pwm_ciclo < brillo[0]) {
        P2 &= ~(1 << 0);   /* Bit 0 = 0 → sink activo */
    } else {
        P2 |= (1 << 0);    /* Bit 0 = 1 → pull-up, LED apagado */
    }

    /* === COMPARACIÓN PWM LED1 (P2.1) === */
    if (pwm_ciclo < brillo[1]) {
        P2 &= ~(1 << 1);
    } else {
        P2 |= (1 << 1);
    }

    /* === COMPARACIÓN PWM LED2 (P2.2) === */
    if (pwm_ciclo < brillo[2]) {
        P2 &= ~(1 << 2);
    } else {
        P2 |= (1 << 2);
    }

    /* === COMPARACIÓN PWM LED3 (P2.3) === */
    if (pwm_ciclo < brillo[3]) {
        P2 &= ~(1 << 3);
    } else {
        P2 |= (1 << 3);
    }

    /* === COMPARACIÓN PWM LED4 (P2.4) === */
    if (pwm_ciclo < brillo[4]) {
        P2 &= ~(1 << 4);
    } else {
        P2 |= (1 << 4);
    }

    /* Incrementar contador de ciclo. Overflow 255→0 es automático en uint8_t */
    pwm_ciclo++;

    /* Cuando pwm_ciclo vuelve a 0, han pasado 256 ticks × 4µs ≈ 1.024ms */
    if (pwm_ciclo == 0) {
        tick_ms++;             /* Contador de milisegundos derivados del PWM */
        if (tick_ms >= 20) {   /* Cada 20ms (50Hz) actualizar efecto visual */
            tick_ms = 0;       /* Resetear contador local */
            flag_efecto = 1;   /* Señalizar al superloop que debe calcular nuevo brillo */
        }
    }
}

/**
 * @brief Efecto "Ola" adaptado para 5 LEDs
 * Una onda de brillo recorre los LEDs de izquierda a derecha y viceversa.
 * Original usaba fase*2 y offsets 33/66 para 3 LEDs.
 * Adaptación: desfase de 51 unidades entre LEDs (255/5≈51).
 * Cada LED tiene su propia fase desplazada, creando ilusión de movimiento.
 */
static void efecto_ola(void) {
    static uint8_t fase_global = 0;  /* Posición base de la ola: 0-255 */
    uint8_t i;                       /* Índice de LED (0-4) */
    uint8_t fase_led;                /* Fase individual de cada LED */
    uint8_t valor_triangular;        /* Valor de brillo calculado (forma triangular) */

    for (i = 0; i < 5; i++) {
        /* Calcular fase individual con desfase de 51 unidades por LED */
        /* LED0=fase, LED1=fase+51, LED2=fase+102, etc. */
        /* Suma con wrap-around natural de uint8_t (255+1=0) */
        fase_led = fase_global + (i * 51);

        /* Convertir fase lineal a forma triangular (sube y baja) */
        /* Si fase_led < 128: brillo sube proporcionalmente (0→255) */
        /* Si fase_led >= 128: brillo baja proporcionalmente (255→0) */
        if (fase_led < 128) {
            valor_triangular = fase_led * 2;         /* 0-127 → 0-254 */
        } else {
            valor_triangular = (255 - fase_led) * 2; /* 128-255 → 254-0 */
        }

        /* Asignar brillo calculado al LED correspondiente */
        brillo[i] = valor_triangular;
    }

    /* Avanzar fase global. Velocidad controlada por frecuencia de llamada (20ms) */
    fase_global += 3;  /* Paso de 3 → ciclo completo en ~85 ticks ≈ 1.7 segundos */
    /* No necesita clamp: overflow uint8_t hace wrap-around natural */
}
/**
 * @brief Efecto "Latido Sincronizado" adaptado para 5 LEDs
 * Todos los LEDs laten al unísono con patrón asimétrico: subida rápida, bajada lenta.
 * Ciclo de 100 ticks: pico en tick 25 (25% subida, 75% bajada).
 * CORREGIDO: Cálculos intermedios en uint16_t para evitar overflow silencioso de uint8_t.
 */
static void efecto_latido(void) {
    static uint8_t fase = 0;     /* Progreso del ciclo de latido: 0-99 */
    uint16_t calculo_temp;       /* ⚠️ uint16_t para cálculo intermedio seguro */
    uint8_t i;                   /* Índice de LED */

    /* Calcular brillo común para todos los LEDs según fase actual */
    if (fase < 25) {
        /* Subida rápida: 0→255 en 25 ticks → paso = 255/25 ≈ 10.2 */
        calculo_temp = (uint16_t)fase * 10;   /* Máximo: 24×10 = 240 → cabe en uint16_t */
        if (calculo_temp > 255) calculo_temp = 255;  /* ✅ Clamp AHORA sí funciona */
    } else {
        /* Bajada lenta: 255→0 en 75 ticks */
        calculo_temp = (uint16_t)((99 - fase) * 3) + 33;  /* Máx: 74×3+33 = 255 */
        if (calculo_temp > 255) calculo_temp = 255;       /* ✅ Clamp AHORA sí funciona */
    }

    /* Asignar MISMO brillo a los 5 LEDs (latido sincronizado) */
    for (i = 0; i < 5; i++) {
        brillo[i] = (uint8_t)calculo_temp;  /* Conversión explícita tras clamp seguro */
    }

    fase++;
    if (fase >= 100) fase = 0;
}

/**
 * @brief Efecto "Ráfaga Secuencial" adaptado para 5 LEDs
 * Pulso brillante que viaja de LED0→LED4 secuencialmente, con decaimiento suave.
 * Este efecto JUSTIFICA tener 5 LEDs: la secuencia espacial es visible.
 * Original: pos<10→max, pos<20→decaimiento, else→0. Escala 0-99.
 * Adaptación: escala 0-255, pulso de 8 ticks, decaimiento de 12 ticks, reposo 30 ticks.
 * Desfase entre LEDs: 50 ticks (ciclo total 250 ticks = 5 segundos).
 */
static void efecto_rafaga(void) {
    static uint8_t fase_global = 0;  /* Posición base de la ráfaga: 0-249 */
    uint8_t i;                       /* Índice de LED (0-4) */
    uint8_t pos_led;                 /* Posición individual de cada LED en el ciclo */

    for (i = 0; i < 5; i++) {
        /* Calcular posición individual con desfase de 50 ticks por LED */
        /* LED0=fase, LED1=fase+50, LED2=fase+100, LED3=fase+150, LED4=fase+200 */
        /* Módulo 250 asegura wrap-around dentro del ciclo completo */
        pos_led = (fase_global + (i * 50)) % 250;

        /* Determinar brillo según posición en el perfil de ráfaga */
        if (pos_led < 8) {
            /* Fase de pulso máximo: primeros 8 ticks a brillo pleno */
            brillo[i] = 255;
        } else if (pos_led < 20) {
            /* Fase de decaimiento: 255→0 en 12 ticks → paso ≈ 21.25, usamos 21 */
            brillo[i] = 255 - ((pos_led - 8) * 21);
            /* (pos_led-8) va de 0→11, ×21 = 0→231. 255-231=24 residual mínimo */
        } else {
            /* Fase de reposo: LED apagado hasta siguiente ráfaga */
            brillo[i] = 0;
        }
    }

    /* Avanzar fase global con wrap-around en 250 (no potencia de 2, requiere módulo o if) */
    fase_global++;
    if (fase_global >= 250) fase_global = 0;  /* Ciclo completo cada 250 ticks (5 segundos) */
}

void main(void) {
    /* === CONFIGURACIÓN P2.0-P2.4 COMO SALIDAS CUASI-BIDIRECCIONALES === */
    /* Máscara 0x1F = bits 0-4 en 1, bits 5-7 en 0 → configura 5 pines simultáneamente */
    P2M1 &= ~0x1F;               /* M1[4:0] = 00000 */
    P2M0 &= ~0x1F;               /* M0[4:0] = 00000 → Cuasi-bidireccional */
    P2 |= 0x1F;                  /* Todos los LEDs apagados inicialmente (pull-ups activos) */

    /* === CONFIGURACIÓN TIMER0 MODO 16-BIT AUTO-RELOAD 4µS === */
    AUXR &= ~(1 << 7);           /* ⚠️ CRÍTICO: Forzar modo 12T → FOSC/12 = 2MHz */
    TMOD &= ~0x0F;               /* Modo 0 STC8H = 16-bit auto-reload */

    /* Recarga para 4µs @ 2MHz: 8 ticks → 65536-8 = 65528 = 0xFFF8 */
    TH0 = 0xFF;                  /* Byte alto de recarga */
    TL0 = 0xF8;                  /* Byte bajo de recarga */

    ET0 = 1;                     /* Habilitar interrupción específica Timer0 (IE.1) */
    EA = 1;                      /* Habilitar interrupciones globales (IE.7) */
    TR0 = 1;                     /* Arrancar Timer0 (TCON.4) */

    /* === SUPERLOOP: Solo ejecuta efecto cuando ISR marca flag === */
    while (1) {
        if (flag_efecto == 1) {  /* Esperar señal de ISR (cada 20ms) */
            flag_efecto = 0;     /* Limpiar flag inmediatamente tras leer */

            /* SELECCIONAR EFECTO: Descomentar UNO solo, comentar los demás */
            //efecto_ola();        /* Onda viajera bidireccional */
             efecto_latido();  /* Latido sincronizado asimétrico */
            // efecto_rafaga();  /* Pulso secuencial LED0→LED4 */
        }
    }
}
