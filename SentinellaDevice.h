#ifndef SENTINELLA_DEVICE_H
#define SENTINELLA_DEVICE_H

#include "Device.h"
#include "Led.h"
#include "Buzzer.h"
#include "Mpu6050Sensor.h"
#include "RainSensor.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>

class SentinellaDevice : public Device {
private:
    Mpu6050Sensor tiltSensor;
    RainSensor rainSensor;
    Led redLed;
    Led greenLed;
    Buzzer buzzer;

    int tiltSeverity;  // 0 = normal, 1 = critical
    int rainSeverity;  // 0 = normal, 1 = critical
    bool lastCritical;
    bool wifiConnected;

    void evaluateGlobalState();
    void postReading(bool critical);

public:
    static const int PIN_TILT_SDA  = 21;
    static const int PIN_RAIN      = 34;
    static const int PIN_LED_RED   = 18;
    static const int PIN_LED_GREEN = 19;
    static const int PIN_BUZZER    = 5;

    SentinellaDevice();

    void begin();
    void update();

    void on(Event event) override;
    void handle(Command command) override;

    float getLatestTilt()    const { return tiltSensor.getLatestTiltDeg(); }
    int   getLatestRainADC() const { return rainSensor.getLatestADC(); }
};

#endif // SENTINELLA_DEVICE_H
