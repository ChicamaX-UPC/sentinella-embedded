#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include "Sensor.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

class Mpu6050Sensor : public Sensor {
private:
    Adafruit_MPU6050 mpu;
    unsigned long lastReadTime;
    float lastTiltDeg;
    int currentState;

public:
    static const int TILT_NORMAL_EVENT_ID = 40;
    static const int TILT_CRITICAL_EVENT_ID = 41;
    static const Event TILT_NORMAL_EVENT;
    static const Event TILT_CRITICAL_EVENT;

    static constexpr float TILT_THRESHOLD = 15.0f; // degrees

    Mpu6050Sensor(int pin, EventHandler* eventHandler = nullptr);
    bool begin();
    float getLatestTiltDeg() const { return lastTiltDeg; }
    void update();
};

#endif // MPU6050_SENSOR_H
