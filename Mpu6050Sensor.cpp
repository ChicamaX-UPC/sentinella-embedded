#include "Mpu6050Sensor.h"

const Event Mpu6050Sensor::TILT_NORMAL_EVENT = Event(TILT_NORMAL_EVENT_ID);
const Event Mpu6050Sensor::TILT_CRITICAL_EVENT = Event(TILT_CRITICAL_EVENT_ID);

Mpu6050Sensor::Mpu6050Sensor(int pin, EventHandler* eventHandler)
    : Sensor(pin, eventHandler), lastReadTime(0), lastTiltDeg(0.0f), currentState(0) {}

bool Mpu6050Sensor::begin() {
    Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
    if (!mpu.begin()) {
        return false;
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    return true;
}

void Mpu6050Sensor::update() {
    unsigned long now = millis();
    if (now - lastReadTime >= 500) {
        lastReadTime = now;

        sensors_event_t accel, gyro, temp;
        mpu.getEvent(&accel, &gyro, &temp);

        float ax = accel.acceleration.x;
        float ay = accel.acceleration.y;
        float az = accel.acceleration.z;
        lastTiltDeg = atan2(sqrt(ax * ax + ay * ay), az) * 180.0f / PI;

        int newState = (lastTiltDeg > TILT_THRESHOLD) ? 1 : 0;

        if (newState != currentState && handler != nullptr) {
            currentState = newState;
            if (currentState == 1) handler->on(TILT_CRITICAL_EVENT);
            else handler->on(TILT_NORMAL_EVENT);
        }
    }
}
