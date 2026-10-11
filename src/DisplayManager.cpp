#include "DisplayManager.h"
#include <Wire.h>

DisplayManager::DisplayManager() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN) {}

bool DisplayManager::begin() {
    Wire.begin();
    // Inisialisasi OLED SSD1306 I2C
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        return false;
    }
    display.setRotation(OLED_ROTATION); // Balik tampilan 180 derajat
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    return true;
}

void DisplayManager::showBootScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(14, 4);
    display.print(F("CR-V GEN 3 ECT"));
    display.setCursor(10, 18);
    display.print(F("FAN CONTROLLER"));
    display.display();
    delay(1200);
}

void DisplayManager::renderFailSafeAlert(const SensorData& sensor) {
    // Mode Peringatan Bahaya (Fail-safe)
    display.fillRect(0, 0, SCREEN_WIDTH, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(12, 2);
    display.print(F("! SENSOR ERROR !"));

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.print(F("FAN : [FORCED ON]"));
    display.setCursor(0, 24);
    if (sensor.isOpenCircuit) {
        display.print(F("Status: KABEL PUTUS"));
    } else {
        display.print(F("Status: SHORT TO GND"));
    }
}

void DisplayManager::renderDashboard(const SensorData& sensor, const SystemSettings& settings, bool isFanOn) {
    if (!sensor.isValid) {
        renderFailSafeAlert(sensor);
        return;
    }

    // --- SISI KIRI: DISPLAY SUHU BESAR ---
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 8);

    // Format suhu: Jika >= 100 tampilkan tanpa desimal agar pas, < 100 tampilkan 1 desimal
    if (sensor.temperature >= 100.0f) {
        display.print((int)sensor.temperature);
    } else {
        display.print(sensor.temperature, 1);
    }
    
    // Simbol derajat celcius
    display.setTextSize(1);
    display.print((char)247);
    display.print(F("C"));

    // Garis pemisah vertikal
    display.drawFastVLine(63, 0, SCREEN_HEIGHT, SSD1306_WHITE);

    // --- SISI KANAN: STATUS FAN & SETPOINTS ---
    if (isFanOn) {
        // Kotak blok putih dengan teks hitam jika fan menyala
        display.fillRect(67, 1, 60, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(72, 2);
        display.print(F("FAN ON"));
    } else {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(72, 2);
        display.print(F("FAN OFF"));
    }

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(67, 13);
    display.print(F("ON :"));
    display.print(settings.tempUpper);
    display.print(F("C"));

    display.setCursor(67, 23);
    display.print(F("OFF:"));
    display.print(settings.tempLower);
    display.print(F("C"));
}

void DisplayManager::renderMenuUpper(const SystemSettings& settings) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("SET SUHU FAN ON"));

    display.setTextSize(2);
    display.setCursor(20, 12);
    display.print(F("[ "));
    display.print(settings.tempUpper);
    display.print(F(" C ]"));
}

void DisplayManager::renderMenuLower(const SystemSettings& settings) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("SET SUHU FAN OFF"));

    display.setTextSize(2);
    display.setCursor(20, 12);
    display.print(F("[ "));
    display.print(settings.tempLower);
    display.print(F(" C ]"));
}

void DisplayManager::renderMenuOffset(uint8_t zoneIndex, int16_t offsetTenths, const SensorData& sensor) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (zoneIndex == 1) {
        display.print(F("CAL1:40-60C "));
    } else if (zoneIndex == 2) {
        display.print(F("CAL2:60-80C "));
    } else {
        display.print(F("CAL3:>80C   "));
    }

    display.print(F("T:"));
    if (sensor.isValid) {
        if (sensor.temperature >= 100.0f) {
            display.print((int)sensor.temperature);
        } else {
            display.print(sensor.temperature, 1);
        }
        display.print((char)247);
        display.print(F("C"));
    } else {
        display.print(F("ERR"));
    }

    display.setTextSize(2);
    display.setCursor(10, 12);
    display.print(F("["));
    if (offsetTenths > 0) {
        display.print(F("+"));
    } else if (offsetTenths < 0) {
        display.print(F("-"));
    } else {
        display.print(F(" "));
    }
    int16_t absVal = abs(offsetTenths);
    if (absVal < 100) {
        display.print(F(" "));
    }
    display.print(absVal / 10);
    display.print(F("."));
    display.print(absVal % 10);
    display.print(F(" C]"));
}

void DisplayManager::update(MenuState menuState, const SensorData& sensor, const SystemSettings& settings, bool isFanOn) {
    display.clearDisplay();

    switch (menuState) {
        case MENU_DASHBOARD:
            renderDashboard(sensor, settings, isFanOn);
            break;
        case MENU_SET_TEMP_UPPER:
            renderMenuUpper(settings);
            break;
        case MENU_SET_TEMP_LOWER:
            renderMenuLower(settings);
            break;
        case MENU_CALIBRATE_OFFSET_1:
            renderMenuOffset(1, settings.tempOffset1, sensor);
            break;
        case MENU_CALIBRATE_OFFSET_2:
            renderMenuOffset(2, settings.tempOffset2, sensor);
            break;
        case MENU_CALIBRATE_OFFSET_3:
            renderMenuOffset(3, settings.tempOffset3, sensor);
            break;
    }

    display.display();
}
