#ifndef BUZZER_H
#define BUZZER_H

#include "Actuator.h"

class Buzzer : public Actuator {
private:
    bool isPlaying;
    unsigned long noteStartTime;
    int currentNote;

public:
    static const int PLAY_ALARM_COMMAND_ID = 10;
    static const int STOP_ALARM_COMMAND_ID = 11;
    static const Command PLAY_ALARM_COMMAND;
    static const Command STOP_ALARM_COMMAND;

    Buzzer(int pin, CommandHandler* commandHandler = nullptr);

    /// Configura el pin como salida. Llamar desde setup()/begin(), no desde el constructor.
    void begin();

    void handle(Command command) override;
    void update();
};

#endif