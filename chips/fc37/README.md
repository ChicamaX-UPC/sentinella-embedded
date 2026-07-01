# Custom chip Wokwi — FC-37 (YL-83) raindrop sensor

Modela el módulo pluviométrico **FC-37** (placa + comparador LM393), que Wokwi no trae como
pieza nativa. Reemplaza al potenciómetro que antes simulaba la lluvia.

## Pinout

| Pin | Descripción |
|-----|-------------|
| `VCC` | Alimentación (3V3) |
| `GND` | Tierra |
| `AO`  | Salida **analógica**, INVERTIDA: seco → ~VCC (ADC ~4095), lluvia → ~0 V (ADC ~0) |
| `DO`  | Salida **digital** activo-bajo: `LOW` cuando la lluvia supera el umbral del módulo |

Cableado en `diagram.json`: `AO→esp:34` (ADC), `DO→esp:35`, `VCC→3V3`, `GND→GND`.

## Comportamiento

- Control deslizante **"Lluvia (%)"** (0–100) en el simulador.
- `V_AO = VCC · (1 − lluvia/100)` → el firmware calcula `rain_pct = (4095 − AO)/40.95`.
- `DO = LOW` cuando `lluvia ≥ 50%` (umbral interno `DO_THRESHOLD_PCT`, análogo al trimpot real).
- El firmware marca **CRITICAL** cuando `rain_pct > 70%` (`RainSensor::RAIN_CRITICAL_PCT`).

## Cómo usarlo

### Opción 1 — Wokwi online (recomendada, sin toolchain)
El editor de chips de wokwi.com compila el C a WASM en el navegador:
1. Abre el proyecto en https://wokwi.com y crea un **Custom Chip** llamado `fc37`.
2. Pega el contenido de [`fc37.chip.json`](fc37.chip.json) y [`fc37.chip.c`](fc37.chip.c).
3. El `diagram.json` ya referencia la pieza como `"type": "chip-fc37"`. Pulsa ▶.

### Opción 2 — VS Code (Wokwi extension, requiere build)
1. Usa la plantilla oficial de custom chips en C (trae `Makefile` y `wokwi-api.h`):
   https://github.com/wokwi/custom-chip-c-example
2. Copia `fc37.chip.c` y `fc37.chip.json`, ejecuta `make` → genera `dist/fc37.chip.wasm`.
3. En `wokwi.toml` del proyecto Sentinella, registra el chip:
   ```toml
   [[chip]]
   name = 'fc37'
   binary = 'chips/fc37/dist/fc37.chip.wasm'
   ```
4. Ejecuta *Wokwi: Start Simulator*.

## Probar (end-to-end)
Sube el deslizante **"Lluvia (%)"** por encima de 70 % para forzar `RAIN_CRITICAL`. El estado
global **CRITICAL** requiere además inclinación > 15° (MPU6050); al cumplirse ambos, se enciende el
LED rojo + buzzer y el dispositivo hace `POST` con `rain_pct` real al edge.
