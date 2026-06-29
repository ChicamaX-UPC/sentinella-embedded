#include "SentinellaDevice.h"
#include <Arduino.h>

SentinellaDevice::SentinellaDevice()
    : tiltSensor(PIN_TILT_SDA, this),
      rainSensor(PIN_RAIN, this),
      redLed(PIN_LED_RED, false, this),
      greenLed(PIN_LED_GREEN, true, this),
      buzzer(PIN_BUZZER, this),
      tiltSeverity(0), rainSeverity(0), lastCritical(false) {}

void SentinellaDevice::begin() {
    Serial.println("\n=== Sentinella Edge Node - Tailing Dam Monitor ===");
    if (!tiltSensor.begin()) {
        Serial.println("[ERROR] MPU6050 no encontrado — revisa SDA/SCL.");
        while (true) delay(10);
    }
    Serial.println("[INFO]  MPU6050 inicializado.");
    Serial.println("[INFO]  Polling cada 500 ms — sin delay().\n");
}

void SentinellaDevice::on(Event event) {
    if (event == Mpu6050Sensor::TILT_CRITICAL_EVENT)   tiltSeverity = 1;
    else if (event == Mpu6050Sensor::TILT_NORMAL_EVENT) tiltSeverity = 0;
    else if (event == RainSensor::RAIN_CRITICAL_EVENT)  rainSeverity = 1;
    else if (event == RainSensor::RAIN_NORMAL_EVENT)    rainSeverity = 0;
}

void SentinellaDevice::handle(Command command) {
    // No external commands for this edge device
}

void SentinellaDevice::evaluateGlobalState() {
    bool critical = (tiltSeverity == 1) && (rainSeverity == 1);

    if (critical) {
        redLed.handle(Led::TURN_ON_COMMAND);
        greenLed.handle(Led::TURN_OFF_COMMAND);
        buzzer.handle(Buzzer::PLAY_ALARM_COMMAND);
    } else {
        redLed.handle(Led::TURN_OFF_COMMAND);
        greenLed.handle(Led::TURN_ON_COMMAND);
        buzzer.handle(Buzzer::STOP_ALARM_COMMAND);
    }

    if (critical != lastCritical) {
        lastCritical = critical;
        const char* label = critical ? "[CRITICAL]" : "[NORMAL]  ";
        Serial.printf("%s  Tilt: %6.2f°  |  Rain ADC: %4d / 4095  (%.1f%%)\n",
                      label, getLatestTilt(), getLatestRainADC(), getLatestRainADC() / 40.95f);
    }
}

void SentinellaDevice::update() {
    tiltSensor.update();
    rainSensor.update();
    buzzer.update();
    evaluateGlobalState();
}
