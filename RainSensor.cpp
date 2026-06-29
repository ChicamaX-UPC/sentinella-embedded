#include "RainSensor.h"
#include <Arduino.h>

const Event RainSensor::RAIN_NORMAL_EVENT = Event(RAIN_NORMAL_EVENT_ID);
const Event RainSensor::RAIN_CRITICAL_EVENT = Event(RAIN_CRITICAL_EVENT_ID);

RainSensor::RainSensor(int pin, EventHandler* eventHandler)
    : Sensor(pin, eventHandler), lastReadTime(0), lastADC(0), currentState(0) {
    pinMode(pin, INPUT);
}

void RainSensor::update() {
    unsigned long now = millis();
    if (now - lastReadTime >= 500) {
        lastReadTime = now;

        lastADC = analogRead(pin);
        int newState = (lastADC > RAIN_THRESHOLD) ? 1 : 0;

        if (newState != currentState && handler != nullptr) {
            currentState = newState;
            if (currentState == 1) handler->on(RAIN_CRITICAL_EVENT);
            else handler->on(RAIN_NORMAL_EVENT);
        }
    }
}
