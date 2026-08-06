# 06pwmSoftware - STC8H8K64U Bare Metal

PWM por software con desvanecimiento suave (breathing) para LED en P2.0.  
Implementado con Timer0 e ISR en SDCC sobre Linux para STC8H8K64U @ 24MHz.

## Filosofía de Trabajo

- **Cero abstracciones:** Sin HAL, Arduino ni librerías PWM específicas.
- **PWM por software transparente:** Generación visible y depurable en ISR.
- **Sintaxis SDCC pura:** `__sfr __at()` y `__sbit __at()` en lugar de sintaxis Keil.
- **ISR mínima:** Solo comparación + conteo base tiempo. Lógica de efecto en superloop.
- **Un solo timer:** Timer0 genera PWM (~1kHz) Y base de tiempo para desvanecimiento (~10ms).
- **Verificación primaria:** Direcciones SFR validadas contra Reference Manual oficial (2022/3/9).
- **Ambiente 100% Linux:** SDCC + stcgal + Makefile + Bash.

## Hardware

- MCU: STC8H8K64U @ 24MHz (configuración de fábrica, modo 12T forzado explícitamente)
- LED: P2.0 en configuración sink (5V → LED → R → Pin), modo cuasi-bidireccional
- Programador: Adaptador USB-TTL PL2303 conectado a UART0 del MCU
- Clock Timer0: FOSC/12 = 2MHz (AUXR.T0x12 = 0 forzado)

## Comportamiento

- LED realiza ciclo de desvanecimiento: brillo sube de 0→255 y baja de 255→0 suavemente
- Frecuencia PWM: ~1kHz (período 1.024ms, 256 pasos de 4µs)
- Velocidad de desvanecimiento: ~2.5 segundos ida, ~2.5 segundos vuelta
- Resolución: 256 niveles de brillo (8 bits nativos)

## Estructura del Proyecto

06pwmSoftware/
├── 06pwmSoftware.c   # Código principal con ISR PWM + desvanecimiento
├── stc8h.h                     # Direcciones SFR verificadas (P2 + Timer0 + IE)
├── Makefile                    # Compilación y grabación vía PL2303
├── README.md                   # Este archivo
├── License.txt                 # UNLICENSE (dominio público, bilingüe)
└── .gitignore                  # Exclusión de binarios y temporales


## Requisitos

- SDCC (>= 4.0 recomendado)
- stcgal (>= 1.7 con soporte STC8G/8H)
- Adaptador USB-TTL PL2303 (o compatible)
- Permisos de acceso a `/dev/ttyUSB0` (ajustar en Makefile si es diferente)

## Uso

# Compilar
make

# Grabar en el microcontrolador vía PL2303
make flash

# Limpiar archivos generados
make clean

Notas Técnicas

    PWM por software: La ISR compara pwm_ciclo (0-255) vs brillo_actual cada 4µs. Si ciclo < brillo → LED ON; sino → LED OFF. Duty cycle proporcional al valor de brillo.
    Overflow natural de uint8_t: pwm_ciclo++ se desborda de 255→0 automáticamente. No requiere reset manual. Eficiente y transparente.
    Base de tiempo derivada: El mismo overflow de pwm_ciclo (cada 256 ticks ≈ 1ms) cuenta 10 ciclos para actualizar brillo cada ~10ms. Un solo timer, dos funciones.
    AUXR.T0x12 = 0 forzado: Garantiza modo 12T independientemente del estado previo del chip. Nunca confiar en defaults.
    Recarga Timer0: 4µs @ 2MHz = 8 ticks → 65536-8 = 65528 = 0xFFF8.
    ISR mínima: Solo comparación, incremento y chequeo de contador. Cero llamadas a función, cero lógica compleja.

Referencias

    STC8H Reference Manual (2022/3/9)
    SDCC Compiler User Guide
    stcgal Documentation

