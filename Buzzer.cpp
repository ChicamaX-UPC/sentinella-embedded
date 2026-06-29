#include "Buzzer.h"
#include <Arduino.h>

const Command Buzzer::PLAY_ALARM_COMMAND = Command(PLAY_ALARM_COMMAND_ID);
const Command Buzzer::STOP_ALARM_COMMAND = Command(STOP_ALARM_COMMAND_ID);

#define BPM 175
#define DURATION          (int)(60000 / BPM)
#define DURATION_HALF_DOT (int)(45000 / BPM)
#define DURATION_HALF     (int)(30000 / BPM)
#define DURATION_QUARTER  (int)(15000 / BPM)

#define NOTE_DS5 622
#define NOTE_B4  494
#define NOTE_AS4 466
#define NOTE_A4  440

static const int melody[] = {
    NOTE_B4, NOTE_B4, NOTE_DS5, NOTE_A4, 0, NOTE_DS5, NOTE_AS4,
    0, NOTE_DS5, NOTE_AS4, 0, NOTE_DS5, NOTE_AS4,
    0, NOTE_DS5, NOTE_AS4, 0, NOTE_DS5, NOTE_AS4,
    0, NOTE_DS5, NOTE_AS4, 0, NOTE_DS5, NOTE_AS4
};

static const int noteDurations[] = {
    DURATION_QUARTER, DURATION_HALF_DOT, DURATION_HALF, DURATION_HALF, DURATION, DURATION_HALF, DURATION_HALF,
    DURATION, DURATION_HALF, DURATION_HALF, DURATION, DURATION_HALF, DURATION_HALF,
    DURATION, DURATION_HALF, DURATION_HALF, DURATION, DURATION_HALF, DURATION_HALF,
    DURATION, DURATION_HALF, DURATION_HALF, DURATION, DURATION_HALF, DURATION_HALF,
};

static const int NUM_NOTES = sizeof(melody) / sizeof(melody[0]);

Buzzer::Buzzer(int pin, CommandHandler* commandHandler)
    : Actuator(pin, commandHandler), isPlaying(false), noteStartTime(0), currentNote(0) {
    pinMode(pin, OUTPUT);
    noTone(pin);
}

void Buzzer::handle(Command command) {
    if (command == PLAY_ALARM_COMMAND) {
        isPlaying = true;
    } else if (command == STOP_ALARM_COMMAND) {
        isPlaying = false;
        noTone(pin);
        currentNote = 0;
    }
    Actuator::handle(command);
}

void Buzzer::update() {
    if (!isPlaying) return;

    unsigned long now = millis();
    int duration = noteDurations[currentNote];

    if (now - noteStartTime >= (duration * 1.1)) {
        noteStartTime = now;
        if (melody[currentNote] > 0) {
            tone(pin, melody[currentNote], duration);
        } else {
            noTone(pin);
        }
        currentNote++;
        if (currentNote >= NUM_NOTES) {
            currentNote = 0;
        }
    }
}
