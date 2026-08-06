# 07pwmSoftware_Efectos - STC8H8K64U Bare Metal

Tres efectos de luz (Ola, Latido, Ráfaga) para 5 LEDs con PWM por software.  
Implementado con Timer0 e ISR en SDCC sobre Linux para STC8H8K64U @ 24MHz.

## Filosofía de Trabajo

- **Cero abstracciones:** Sin HAL, Arduino ni librerías PWM/LED específicas.
- **PWM por software transparente:** Generación visible y depurable en ISR.
- **ISR desrollada:** 5 comparaciones explícitas, sin bucles ni indexación dinámica.
- **Tipos seguros:** Cálculos intermedios en `uint16_t` para evitar overflow silencioso.
- **Un solo timer:** Timer0 genera PWM (~500Hz) Y base de tiempo para efectos (~40ms).
- **Verificación primaria:** Direcciones SFR validadas contra Reference Manual oficial (2022/3/9).
- **Ambiente 100% Linux:** SDCC + stcgal + Makefile + Bash.

## Hardware

- MCU: STC8H8K64U @ 24MHz (modo 12T forzado explícitamente vía AUXR.T0x12=0)
- LEDs: P2.0-P2.4 en configuración sink (5V → LED → R → Pin), modo cuasi-bidireccional
- Programador: Adaptador USB-TTL PL2303 conectado a UART0 del MCU
- Clock Timer0: FOSC/12 = 2MHz

## Efectos Disponibles

| Efecto | Descripción | Patrón Temporal |
|--------|-------------|-----------------|
| **Ola** | Onda triangular viajera LED0→LED4→LED0 | Ciclo ~1.7s, desfase 51 unidades entre LEDs |
| **Latido** | Latido asimétrico sincronizado (subida rápida/bajada lenta) | Ciclo 2s, pico en 25% del ciclo |
| **Ráfaga** | Pulso secuencial LED0→LED4 con decaimiento | Ciclo 5s, pulso 8 ticks + decaimiento 12 ticks |

Seleccionar efecto descomentando UNA línea en el superloop de `main()`.

## Estructura del Proyecto

07pwmSoftware_Efectos/
├── 07pwmSoftware_Efectos.c     # Código principal con ISR PWM + 3 efectos
├── stc8h.h                 # Direcciones SFR verificadas (P2 + Timer0 + IE)
├── Makefile                # Compilación y grabación vía PL2303
├── README.md               # Este archivo
├── License.txt             # UNLICENSE (dominio público, bilingüe)
└── .gitignore              # Exclusión de binarios y temporales


## Requisitos

- SDCC (>= 4.0 recomendado)
- stcgal (>= 1.7 con soporte STC8G/8H)
- Adaptador USB-TTL PL2303 (o compatible)
- Permisos de acceso a `/dev/ttyUSB0` (ajustar en Makefile si es diferente)

## Uso

make          # Compilar
make flash    # Grabar vía PL2303
make clean    # Limpiar artefactos

Notas Técnicas

    ISR desrollada: 5 comparaciones if (pwm_ciclo < brillo[n]) generan ~15 ciclos máquina. Sin bucles, sin punteros, sin indexación dinámica. Latencia predecible.
    Timing ajustado: 8µs por tick (recarga 0xFFF0) da margen seguro para 5 comparaciones. PWM resultante ~500Hz (invisible al ojo), refresh efectos 40ms (25Hz, suave).
    uint16_t para cálculos: Evita warning 94/126 de SDCC. Clamp funcional tras cálculo intermedio seguro. Conversión explícita a uint8_t tras validar rango.
    Desfase espacial: Ola usa offset 51 (255/5), Ráfaga usa offset 50 ticks (250/5). Ambos cubren exactamente el rango/ciclo sin residuos.
    Módulo solo en superloop: (fase + offset) % 250 en efecto Ráfaga está en superloop, NUNCA en ISR. En 8051 compila a resta condicional, no división hardware.
    Configuración masiva GPIO: P2M1 &= ~0x1F configura 5 pines en UNA operación read-modify-write.

    Referencias

    STC8H Reference Manual (2022/3/9)
    SDCC Compiler User Guide
    stcgal Documentation
