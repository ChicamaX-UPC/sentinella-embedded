#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include "Sensor.h"

/**
 * Sensor de lluvia FC-37 (YL-83), módulo con comparador LM393.
 *
 * AO (analógico) está INVERTIDO respecto a la intensidad de lluvia:
 *   seco  -> AO alto (ADC ~4095)   |   lluvia -> AO bajo (ADC ~0)
 * Por eso el porcentaje de lluvia se calcula como (4095 - AO) / 40.95.
 * DO (digital) es activo-bajo: LOW cuando el módulo detecta lluvia.
 */
class RainSensor : public Sensor {
private:
    unsigned long lastReadTime;
    int   lastAO;        ///< Lectura cruda de AO (0-4095): seco≈4095, mojado≈0.
    float lastRainPct;   ///< Porcentaje de lluvia (0-100), derivado de AO invertido.
    int   doPin;         ///< Pin de la salida digital DO (-1 si no se cablea).
    bool  lastRaining;   ///< Estado de DO (true = lluvia detectada, DO en LOW).
    int   currentState;

public:
    static const int RAIN_NORMAL_EVENT_ID = 50;
    static const int RAIN_CRITICAL_EVENT_ID = 51;
    static const Event RAIN_NORMAL_EVENT;
    static const Event RAIN_CRITICAL_EVENT;

    /// Umbral de criticidad por porcentaje de lluvia (AO invertido).
    static constexpr float RAIN_CRITICAL_PCT = 70.0f;

    /**
     * @param aoPin  Pin ADC conectado a AO del FC-37.
     * @param doPin  Pin conectado a DO del FC-37 (opcional; -1 para omitirlo).
     */
    RainSensor(int aoPin, int doPin = -1, EventHandler* eventHandler = nullptr);

    int   getLatestADC() const { return lastAO; }          ///< AO crudo (payload rain_adc).
    float getLatestRainPct() const { return lastRainPct; } ///< % de lluvia (payload rain_pct).
    bool  isRainingDigital() const { return lastRaining; } ///< Señal digital DO del módulo.
    void  update();
};

#endif // RAIN_SENSOR_H
