#include "Mpu6050Sensor.h"
#include <Arduino.h>

const Event Mpu6050Sensor::TILT_NORMAL_EVENT = Event(TILT_NORMAL_EVENT_ID);
const Event Mpu6050Sensor::TILT_CRITICAL_EVENT = Event(TILT_CRITICAL_EVENT_ID);

Mpu6050Sensor::Mpu6050Sensor(int pin, EventHandler* eventHandler)
    : Sensor(pin, eventHandler), lastReadTime(0), lastTiltDeg(0.0f),
      baselineTiltDeg(0.0f), currentState(0) {}

bool Mpu6050Sensor::begin() {
    Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22

    // Despertar el MPU6050 (registro 0x6B = PWR_MGMT_1)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    uint8_t error = Wire.endTransmission(true);
    if (error != 0) {
        return false; // sin ACK del dispositivo: no responde en el bus
    }

    delay(100);

    // Calibra la linea base con el sensor en reposo, para medir deformacion
    // (desviacion respecto a como quedo instalado) y no angulo absoluto.
    float sum = 0.0f;
    for (int i = 0; i < BASELINE_SAMPLES; i++) {
        int16_t ax, ay, az;
        readAccel(ax, ay, az);
        sum += computeRawTiltDeg(ax, ay, az);
        delay(100);
    }
    baselineTiltDeg = sum / BASELINE_SAMPLES;

    return true;
}

void Mpu6050Sensor::readAccel(int16_t &ax, int16_t &ay, int16_t &az) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // ACCEL_XOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom((int)MPU_ADDR, 6, true);

    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
}

float Mpu6050Sensor::computeRawTiltDeg(int16_t ax, int16_t ay, int16_t az) const {
    return atan2(sqrt((float)ax * ax + (float)ay * ay), (float)az) * 180.0f / PI;
}

void Mpu6050Sensor::update() {
    unsigned long now = millis();
    if (now - lastReadTime >= 500) {
        lastReadTime = now;

        int16_t ax, ay, az;
        readAccel(ax, ay, az);

        float rawTiltDeg = computeRawTiltDeg(ax, ay, az);
        lastTiltDeg = fabs(rawTiltDeg - baselineTiltDeg);

        int newState = (lastTiltDeg > TILT_THRESHOLD) ? 1 : 0;

        if (newState != currentState && handler != nullptr) {
            currentState = newState;
            if (currentState == 1) handler->on(TILT_CRITICAL_EVENT);
            else handler->on(TILT_NORMAL_EVENT);
        }
    }
}
