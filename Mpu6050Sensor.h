#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include "Sensor.h"
#include <Wire.h>
#include <math.h>

/**
 * Lee el MPU6050 por I2C crudo (registros) en vez de la libreria Adafruit_MPU6050,
 * ya que esta ultima valida el registro WHO_AM_I y rechaza varios modulos "clon"
 * que si responden correctamente a la comunicacion I2C.
 */
class Mpu6050Sensor : public Sensor {
private:
    static const uint8_t MPU_ADDR = 0x68;
    static const int BASELINE_SAMPLES = 10;

    unsigned long lastReadTime;
    float lastTiltDeg;   // deformacion = desviacion respecto a la linea base calibrada
    float baselineTiltDeg;
    int currentState;

    void readAccel(int16_t &ax, int16_t &ay, int16_t &az);
    float computeRawTiltDeg(int16_t ax, int16_t ay, int16_t az) const;

public:
    static const int TILT_NORMAL_EVENT_ID = 40;
    static const int TILT_CRITICAL_EVENT_ID = 41;
    static const Event TILT_NORMAL_EVENT;
    static const Event TILT_CRITICAL_EVENT;

    static constexpr float TILT_THRESHOLD = 5.0f; // grados de deformacion respecto a la base

    Mpu6050Sensor(int pin, EventHandler* eventHandler = nullptr);
    bool begin();
    float getLatestTiltDeg() const { return lastTiltDeg; }
    float getBaselineTiltDeg() const { return baselineTiltDeg; }
    void update();
};

#endif // MPU6050_SENSOR_H
