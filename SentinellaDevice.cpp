#include "SentinellaDevice.h"
#include <Arduino.h>

SentinellaDevice::SentinellaDevice()
    : tiltSensor(PIN_TILT_SDA, this),
      rainSensor(PIN_RAIN, PIN_RAIN_DO, this),
      redLed(PIN_LED_RED, false, this),
      greenLed(PIN_LED_GREEN, true, this),
      buzzer(PIN_BUZZER, this),
      tiltSeverity(0), rainSeverity(0), lastCritical(false), wifiConnected(false) {}

void SentinellaDevice::begin() {
    Serial.println("\n=== Sentinella Edge Node - Tailing Dam Monitor ===");

    Serial.printf("[WIFI]  Conectando a %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("\n[WIFI]  Conectado — IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        wifiConnected = false;
        Serial.println("\n[WIFI]  Sin conexión — modo offline.");
    }

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
        Serial.printf("%s  Tilt: %6.2f°  |  Rain AO: %4d / 4095  (%.1f%%)  DO:%s\n",
                      label, getLatestTilt(), getLatestRainADC(), getLatestRainPct(),
                      rainSensor.isRainingDigital() ? "LLUVIA" : "seco");
        postReading(critical);
    }
}

void SentinellaDevice::update() {
    tiltSensor.update();
    rainSensor.update();
    buzzer.update();
    evaluateGlobalState();
}

void SentinellaDevice::postReading(bool critical) {
    if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP]  Sin WiFi — lectura no enviada.");
        return;
    }

    HTTPClient http;
    http.begin(EDGE_URL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", EDGE_API_KEY);

    String payload = "{";
    payload += "\"device_id\":\"" DEVICE_ID "\",";
    payload += "\"tilt_deg\":" + String(getLatestTilt(), 2) + ",";
    payload += "\"rain_adc\":" + String(getLatestRainADC()) + ",";
    payload += "\"rain_pct\":" + String(getLatestRainPct(), 1) + ",";
    payload += "\"status\":\"" + String(critical ? "CRITICAL" : "NORMAL") + "\"";
    payload += "}";

    int code = http.POST(payload);
    if (code > 0) {
        Serial.printf("[HTTP]  POST enviado → %d\n", code);
    } else {
        Serial.printf("[HTTP]  Error: %s\n", http.errorToString(code).c_str());
    }
    http.end();
}
