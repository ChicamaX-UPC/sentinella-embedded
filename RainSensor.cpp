#include "RainSensor.h"
#include <Arduino.h>

const Event RainSensor::RAIN_NORMAL_EVENT = Event(RAIN_NORMAL_EVENT_ID);
const Event RainSensor::RAIN_CRITICAL_EVENT = Event(RAIN_CRITICAL_EVENT_ID);

RainSensor::RainSensor(int aoPin, int doPin, EventHandler* eventHandler)
    : Sensor(aoPin, eventHandler), lastReadTime(0), lastAO(4095), lastRainPct(0.0f),
      doPin(doPin), lastRaining(false), currentState(0) {
}

void RainSensor::begin() {
    pinMode(pin, INPUT);
    if (doPin >= 0) {
        pinMode(doPin, INPUT);
    }
}

void RainSensor::update() {
    unsigned long now = millis();
    if (now - lastReadTime >= 500) {
        lastReadTime = now;

        lastAO = analogRead(pin);
        lastRainPct = (4095 - lastAO) / 40.95f;
        if (lastRainPct < 0.0f)   lastRainPct = 0.0f;
        if (lastRainPct > 100.0f) lastRainPct = 100.0f;

        if (doPin >= 0) {
            lastRaining = (digitalRead(doPin) == LOW);
        }

        int newState = (lastRainPct > RAIN_CRITICAL_PCT) ? 1 : 0;

        if (newState != currentState && handler != nullptr) {
            currentState = newState;
            if (currentState == 1) handler->on(RAIN_CRITICAL_EVENT);
            else handler->on(RAIN_NORMAL_EVENT);
        }
    }
}