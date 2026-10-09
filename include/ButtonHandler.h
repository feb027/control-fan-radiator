#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "Config.h"
#include "Types.h"

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin);
    void begin();
    ButtonEvent update();

private:
    uint8_t buttonPin;
    bool lastSteadyState;
    bool lastFlickerableState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    bool longPressFired;
};

#endif // BUTTON_HANDLER_H
