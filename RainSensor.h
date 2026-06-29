#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include "Sensor.h"

class RainSensor : public Sensor {
private:
    unsigned long lastReadTime;
    int lastADC;
    int currentState;

public:
    static const int RAIN_NORMAL_EVENT_ID = 50;
    static const int RAIN_CRITICAL_EVENT_ID = 51;
    static const Event RAIN_NORMAL_EVENT;
    static const Event RAIN_CRITICAL_EVENT;

    static const int RAIN_THRESHOLD = 2867; // ~70% of 4095 ADC range

    RainSensor(int pin, EventHandler* eventHandler = nullptr);
    int getLatestADC() const { return lastADC; }
    void update();
};

#endif // RAIN_SENSOR_H
