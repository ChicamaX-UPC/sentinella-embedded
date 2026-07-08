#ifndef LED_H
#define LED_H

#include "Actuator.h"

class Led : public Actuator {
private:
    bool state; ///< Current state of the LED (true = ON, false = OFF).

public:
    static const int TOGGLE_LED_COMMAND_ID = 0;
    static const int TURN_ON_COMMAND_ID = 1;
    static const int TURN_OFF_COMMAND_ID = 2;
    static const Command TOGGLE_LED_COMMAND;
    static const Command TURN_ON_COMMAND;
    static const Command TURN_OFF_COMMAND;

    Led(int pin, bool initialState = false, CommandHandler* commandHandler = nullptr);

    /**
     * Configura el pin como salida y aplica el estado inicial. Llamar desde
     * setup()/begin() — nunca desde el constructor: en ESP32, los objetos globales
     * se construyen antes de que el runtime de Arduino/ESP-IDF termine de preparar
     * el hardware (GPIO matrix, periféricos), lo que puede dejar otros buses (p. ej.
     * I2C) en un estado inconsistente.
     */
    void begin();

    void handle(Command command) override;
    bool getState() const;
    void setState(bool newState);
};

#endif // LED_H