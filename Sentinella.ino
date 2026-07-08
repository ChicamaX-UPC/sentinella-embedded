#include "SentinellaDevice.h"

SentinellaDevice sentinella;

void setup() {
    Serial.begin(115200);
    sentinella.begin();
}

void loop() {
    sentinella.update();
}