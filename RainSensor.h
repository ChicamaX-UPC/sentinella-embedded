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
    int   lastAO;
    float lastRainPct;
    int   doPin;
    bool  lastRaining;
    int   currentState;

public:
    static const int RAIN_NORMAL_EVENT_ID = 50;
    static const int RAIN_CRITICAL_EVENT_ID = 51;
    static const Event RAIN_NORMAL_EVENT;
    static const Event RAIN_CRITICAL_EVENT;

    static constexpr float RAIN_CRITICAL_PCT = 50.0f;

    RainSensor(int aoPin, int doPin = -1, EventHandler* eventHandler = nullptr);

    /// Configura los pines de entrada. Llamar desde setup()/begin(), no desde el constructor.
    void begin();

    int   getLatestADC() const { return lastAO; }
    float getLatestRainPct() const { return lastRainPct; }
    bool  isRainingDigital() const { return lastRaining; }
    void  update();
};

#endif // RAIN_SENSOR_H