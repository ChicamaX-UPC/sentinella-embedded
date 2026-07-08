#include "SentinellaDevice.h"
#include <Arduino.h>
#include <WiFiClientSecure.h>

SentinellaDevice::SentinellaDevice()
    : tiltSensor(PIN_TILT_SDA, this),
      rainSensor(PIN_RAIN, -1, this), // sin DO: solo A0 conectado (GND, A0, VCC)
      tiltSeverity(0), rainSeverity(0), lastCritical(false), wifiConnected(false) {}

void SentinellaDevice::begin() {
    Serial.println("\n=== Sentinella Edge Node - Tailing Dam Monitor ===");

    // MPU6050 primero y en aislamiento (igual que el sketch de prueba que funciona):
    // antes de tocar cualquier otro pin/periférico o de arrancar el WiFi, para
    // descartar cualquier interferencia de timing en el bus I2C.
    if (!tiltSensor.begin()) {
        while (true) {
            Serial.println("[ERROR] MPU6050 no encontrado — revisa SDA/SCL.");
            delay(1000);
        }
    }
    Serial.printf("[INFO]  MPU6050 inicializado — linea base: %.2f°\n", tiltSensor.getBaselineTiltDeg());

    rainSensor.begin();

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
    bool critical = (tiltSeverity == 1) || (rainSeverity == 1);

    if (critical != lastCritical) {
        lastCritical = critical;
        const char* label = critical ? "[CRITICAL]" : "[NORMAL]  ";
        Serial.printf("%s  Tilt: %6.2f°  |  Rain AO: %4d / 4095  (%.1f%%)\n",
                      label, getLatestTilt(), getLatestRainADC(), getLatestRainPct());
        postReading(critical);
    }

    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();
    if (now - lastHeartbeat >= 2000) {
        lastHeartbeat = now;
        Serial.printf("[MEAS]  Tilt: %6.2f° (%s)  |  Rain: %5.1f%% (AO:%4d %s)  |  Global: %s\n",
                      getLatestTilt(), tiltSeverity ? "CRIT" : "ok",
                      getLatestRainPct(), getLatestRainADC(),
                      rainSeverity ? "CRIT" : "ok",
                      lastCritical ? "CRITICAL" : "NORMAL");
    }
}

void SentinellaDevice::update() {
    tiltSensor.update();
    rainSensor.update();
    evaluateGlobalState();
}

void SentinellaDevice::postReading(bool critical) {
    if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP]  Sin WiFi — lectura no enviada.");
        return;
    }

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(10000);
    http.setReuse(false);

    WiFiClientSecure secureClient;
    if (String(EDGE_URL).startsWith("https")) {
        secureClient.setInsecure();
        http.begin(secureClient, EDGE_URL);
    } else {
        http.begin(EDGE_URL);
    }
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", EDGE_API_KEY);
    http.addHeader("ngrok-skip-browser-warning", "true");

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