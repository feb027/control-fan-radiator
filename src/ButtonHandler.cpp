#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin) 
    : buttonPin(pin),
      lastSteadyState(HIGH),
      lastFlickerableState(HIGH),
      lastDebounceTime(0),
      pressStartTime(0),
      longPressFired(false) {}

void ButtonHandler::begin() {
    pinMode(buttonPin, INPUT_PULLUP);
    lastSteadyState = digitalRead(buttonPin);
    lastFlickerableState = lastSteadyState;
}

ButtonEvent ButtonHandler::update() {
    ButtonEvent event = BTN_EVENT_NONE;
    int currentState = digitalRead(buttonPin);

    // Cek apakah ada perubahan status pin untuk debounce
    if (currentState != lastFlickerableState) {
        lastDebounceTime = millis();
        lastFlickerableState = currentState;
    }

    // Jika status pin sudah stabil melebihi durasi debounce
    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Deteksi transisi state
        if (lastSteadyState == HIGH && currentState == LOW) {
            // Tombol mulai ditekan
            lastSteadyState = LOW;
            pressStartTime = millis();
            longPressFired = false;
        } else if (lastSteadyState == LOW && currentState == HIGH) {
            // Tombol dilepas
            lastSteadyState = HIGH;
            if (!longPressFired) {
                // Jika belum sempat memicu long press, anggap sebagai short press
                event = BTN_EVENT_SHORT_PRESS;
            }
        } else if (lastSteadyState == LOW && currentState == LOW) {
            // Tombol sedang ditahan
            if (!longPressFired && (millis() - pressStartTime >= BUTTON_LONG_PRESS_MS)) {
                longPressFired = true;
                event = BTN_EVENT_LONG_PRESS;
            }
        }
    }

    return event;
}
