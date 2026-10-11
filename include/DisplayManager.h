#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "Types.h"

class DisplayManager {
public:
    DisplayManager();
    bool begin();
    void update(MenuState menuState, const SensorData& sensor, const SystemSettings& settings, bool isFanOn);
    void showBootScreen();

private:
    Adafruit_SSD1306 display;

    void renderDashboard(const SensorData& sensor, const SystemSettings& settings, bool isFanOn);
    void renderMenuUpper(const SystemSettings& settings);
    void renderMenuLower(const SystemSettings& settings);
    void renderMenuOffset(uint8_t zoneIndex, int16_t offsetTenths, const SensorData& sensor);
    void renderFailSafeAlert(const SensorData& sensor);
};

#endif // DISPLAY_MANAGER_H
