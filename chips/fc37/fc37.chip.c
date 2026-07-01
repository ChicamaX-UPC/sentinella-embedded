// FC-37 (YL-83) raindrop sensor — Wokwi custom chip.
//
// Modela el módulo pluviométrico FC-37 (placa + comparador LM393):
//   - AO (salida analógica): INVERTIDA respecto a la intensidad de lluvia.
//       placa seca      -> alta resistencia -> AO ~ VCC   (ADC alto en el ESP32)
//       lluvia intensa  -> baja resistencia -> AO ~ 0 V   (ADC bajo en el ESP32)
//     V_AO = VCC * (1 - lluvia/100)
//   - DO (salida digital): activo-bajo. LOW cuando la lluvia supera el umbral del
//     trimpot del módulo (DO_THRESHOLD_PCT), HIGH en seco.
//
// El nivel de lluvia se controla en el simulador con el deslizante "Lluvia (%)".

#include "wokwi-api.h"
#include <stdlib.h>

static const float VCC_VOLTS = 3.3f;        // referencia del ADC del ESP32 en Wokwi
static const float DO_THRESHOLD_PCT = 50.0f; // umbral del comparador (trimpot del módulo)

typedef struct {
  pin_t pin_ao;
  pin_t pin_do;
  uint32_t rain_attr;
} chip_state_t;

static void chip_timer_event(void *user_data);

void chip_init(void) {
  chip_state_t *chip = (chip_state_t *) malloc(sizeof(chip_state_t));
  chip->pin_ao = pin_init("AO", ANALOG);
  chip->pin_do = pin_init("DO", OUTPUT);
  chip->rain_attr = attr_init("rain", 0);

  const timer_config_t timer_config = {
    .callback = chip_timer_event,
    .user_data = chip,
  };
  timer_t timer = timer_init(&timer_config);
  timer_start(timer, 100000, true); // refresca cada 100 ms
}

static void chip_timer_event(void *user_data) {
  chip_state_t *chip = (chip_state_t *) user_data;

  float rain = attr_read(chip->rain_attr); // 0..100
  if (rain < 0.0f) rain = 0.0f;
  if (rain > 100.0f) rain = 100.0f;

  // AO invertido: más lluvia -> menor voltaje.
  float ao_voltage = VCC_VOLTS * (1.0f - rain / 100.0f);
  pin_dac_write(chip->pin_ao, ao_voltage);

  // DO activo-bajo: LOW cuando llueve por encima del umbral del módulo.
  pin_write(chip->pin_do, rain >= DO_THRESHOLD_PCT ? LOW : HIGH);
}
