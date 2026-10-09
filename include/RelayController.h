#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include "Config.h"
#include "Types.h"

class RelayController {
public:
    RelayController(uint8_t pin, bool activeLow);
    void begin();
    void update(const SensorData& sensor, const SystemSettings& settings);
    bool isFanOn() const;
    bool isFailSafeActive() const;

private:
    uint8_t relayPin;
    bool isActiveLow;
    bool fanState;
    bool failSafeActive;
    unsigned long lastSwitchTime;

    void setPhysicalPin(bool state);
};

#endif // RELAY_CONTROLLER_H
