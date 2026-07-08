#include "Led.h"
#include <Arduino.h>

const Command Led::TOGGLE_LED_COMMAND = Command(TOGGLE_LED_COMMAND_ID);
const Command Led::TURN_ON_COMMAND = Command(TURN_ON_COMMAND_ID);
const Command Led::TURN_OFF_COMMAND = Command(TURN_OFF_COMMAND_ID);

Led::Led(int pin, bool initialState, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), state(initialState) {
}

void Led::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
}

void Led::handle(Command command) {
    if (command == TOGGLE_LED_COMMAND) {
        state = !state;
        digitalWrite(pin, state);
    } else if (command == TURN_ON_COMMAND) {
        state = true;
        digitalWrite(pin, state);
    } else if (command == TURN_OFF_COMMAND) {
        state = false;
        digitalWrite(pin, state);
    }
    Actuator::handle(command);
}

bool Led::getState() const {
    return state;
}

void Led::setState(bool newState) {
    state = newState;
    digitalWrite(pin, state);
}