#include "RelayController.h"

#define RELAY_MIN_DWELL_MS 2000 // Jeda minimal 2 detik untuk melindungi kontak relay

RelayController::RelayController(uint8_t pin, bool activeLow)
    : relayPin(pin),
      isActiveLow(activeLow),
      fanState(false),
      failSafeActive(false),
      lastSwitchTime(0) {}

void RelayController::begin() {
    pinMode(relayPin, OUTPUT);
    // Posisi awal: Matikan fan dengan aman saat boot
    setPhysicalPin(false);
}

void RelayController::setPhysicalPin(bool state) {
    fanState = state;
    if (isActiveLow) {
        digitalWrite(relayPin, state ? LOW : HIGH);
    } else {
        digitalWrite(relayPin, state ? HIGH : LOW);
    }
}

void RelayController::update(const SensorData& sensor, const SystemSettings& settings) {
    // 1. EVALUASI PRIORITAS FAIL-SAFE (SENSOR RUSAK / LEPAS)
    if (!sensor.isValid) {
        failSafeActive = true;
        if (!fanState) {
            setPhysicalPin(true);
            lastSwitchTime = millis();
        }
        return;
    }

    failSafeActive = false;

    // Lindungi kontak relay dari siklus saklar terlalu cepat (dwell time)
    if (millis() - lastSwitchTime < RELAY_MIN_DWELL_MS) {
        return;
    }

    // 2. LOGIKA HISTERESIS PENGATURAN SUHU
    if (sensor.temperature >= (float)settings.tempUpper) {
        if (!fanState) {
            setPhysicalPin(true);
            lastSwitchTime = millis();
        }
    } else if (sensor.temperature <= (float)settings.tempLower) {
        if (fanState) {
            setPhysicalPin(false);
            lastSwitchTime = millis();
        }
    }
    // Jika berada di antara batas bawah dan batas atas, pertahankan status fan sebelumnya
}

bool RelayController::isFanOn() const {
    return fanState;
}

bool RelayController::isFailSafeActive() const {
    return failSafeActive;
}
