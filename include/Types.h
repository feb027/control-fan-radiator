#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

enum MenuState {
    MENU_DASHBOARD = 0,
    MENU_SET_TEMP_UPPER,
    MENU_SET_TEMP_LOWER,
    MENU_CALIBRATE_OFFSET_1,
    MENU_CALIBRATE_OFFSET_2,
    MENU_CALIBRATE_OFFSET_3,
    MENU_SET_OLED_TIMEOUT
};

enum ButtonEvent {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT_PRESS,
    BTN_EVENT_LONG_PRESS
};

struct SystemSettings {
    uint16_t magicKey;      // Pengenal validasi memori EEPROM
    int8_t tempUpper;       // Ambang batas Fan ON (°C)
    int8_t tempLower;       // Ambang batas Fan OFF (°C)
    int16_t tempOffset1;    // Offset Zona 1 (dalam sepersepuluh °C, misal -100 = -10.0°C)
    int16_t tempOffset2;    // Offset Zona 2 (dalam sepersepuluh °C)
    int16_t tempOffset3;    // Offset Zona 3 (dalam sepersepuluh °C)
    uint8_t oledTimeoutMin; // Waktu tunggu auto-sleep OLED (menit, 1 - 30)
    uint8_t checksum;       // Checksum sederhana untuk integritas data
};

struct SensorData {
    float temperature;      // Suhu terfilter setelah kalibrasi 3-zona (°C)
    float rawTemperature;   // Suhu mentah dari kurva sebelum kalibrasi (°C)
    float currentOffset;    // Nilai offset aktif terinterpolasi (°C)
    float rawVoltage;       // Tegangan analog terukur (V)
    uint16_t rawAdc;        // Nilai ADC terukur (0 - 1023)
    bool isValid;           // True jika pembacaan sensor dalam batas wajar
    bool isShortCircuit;    // True jika sinyal korslet ke ground
    bool isOpenCircuit;     // True jika sinyal putus (tegangan mendekati 5V)
};

#endif // TYPES_H
