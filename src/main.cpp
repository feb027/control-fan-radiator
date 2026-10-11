#include <Arduino.h>
#include "Config.h"
#include "Types.h"
#include "Storage.h"
#include "EctSensor.h"
#include "ButtonHandler.h"
#include "RelayController.h"
#include "DisplayManager.h"

// Inisialisasi modul-modul sistem
Storage storage;
EctSensor ectSensor(PIN_ECT_SENSOR);
ButtonHandler buttonA(PIN_BUTTON_A);
ButtonHandler buttonB(PIN_BUTTON_B);
RelayController relayController(PIN_RELAY_FAN, RELAY_ACTIVE_LOW);
DisplayManager displayManager;

// State navigasi sistem
MenuState currentMenu = MENU_DASHBOARD;
unsigned long lastMenuInteraction = 0;

// Pewaktu non-blocking
unsigned long lastSensorTime = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastRelayTime = 0;
unsigned long lastSerialLogTime = 0;

void handleMenuNavigation() {
    ButtonEvent evtA = buttonA.update();
    ButtonEvent evtB = buttonB.update();

    if (evtA != BTN_EVENT_NONE || evtB != BTN_EVENT_NONE) {
        lastMenuInteraction = millis();
    }

    SystemSettings& settings = storage.getSettings();

    switch (currentMenu) {
        case MENU_DASHBOARD:
            // Tahan Tombol A selama 1 detik untuk masuk ke menu pengaturan
            if (evtA == BTN_EVENT_LONG_PRESS) {
                currentMenu = MENU_SET_TEMP_UPPER;
            }
            break;

        case MENU_SET_TEMP_UPPER:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempUpper < MAX_ALLOWABLE_TEMP_UPPER) {
                    settings.tempUpper++;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempUpper > settings.tempLower + MIN_HYSTERESIS_GAP) {
                    settings.tempUpper--;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan A untuk lanjut ke menu setting suhu bawah
                currentMenu = MENU_SET_TEMP_LOWER;
            } else if (evtB == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan B untuk simpan dan langsung keluar
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;

        case MENU_SET_TEMP_LOWER:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempLower < settings.tempUpper - MIN_HYSTERESIS_GAP) {
                    settings.tempLower++;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempLower > MIN_ALLOWABLE_TEMP_LOWER) {
                    settings.tempLower--;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 1 (40-60°C)
                currentMenu = MENU_CALIBRATE_OFFSET_1;
            } else if (evtB == BTN_EVENT_LONG_PRESS) {
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;

        case MENU_CALIBRATE_OFFSET_1:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset1 < MAX_ALLOWABLE_OFFSET) {
                    settings.tempOffset1 += OFFSET_STEP_TENTHS;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset1 > MIN_ALLOWABLE_OFFSET) {
                    settings.tempOffset1 -= OFFSET_STEP_TENTHS;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 2 (60-80°C)
                currentMenu = MENU_CALIBRATE_OFFSET_2;
            } else if (evtB == BTN_EVENT_LONG_PRESS) {
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;

        case MENU_CALIBRATE_OFFSET_2:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset2 < MAX_ALLOWABLE_OFFSET) {
                    settings.tempOffset2 += OFFSET_STEP_TENTHS;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset2 > MIN_ALLOWABLE_OFFSET) {
                    settings.tempOffset2 -= OFFSET_STEP_TENTHS;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 3 (>80°C)
                currentMenu = MENU_CALIBRATE_OFFSET_3;
            } else if (evtB == BTN_EVENT_LONG_PRESS) {
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;

        case MENU_CALIBRATE_OFFSET_3:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset3 < MAX_ALLOWABLE_OFFSET) {
                    settings.tempOffset3 += OFFSET_STEP_TENTHS;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.tempOffset3 > MIN_ALLOWABLE_OFFSET) {
                    settings.tempOffset3 -= OFFSET_STEP_TENTHS;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS || evtB == BTN_EVENT_LONG_PRESS) {
                // Simpan ke EEPROM dan kembali ke dashboard
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;
    }

    // Auto-timeout: Jika di dalam menu dan tidak ada tombol ditekan selama 10 detik,
    // simpan pengaturan dan otomatis kembali ke tampilan utama
    if (currentMenu != MENU_DASHBOARD && (millis() - lastMenuInteraction > INTERVAL_MENU_TIMEOUT_MS)) {
        storage.saveSettings();
        currentMenu = MENU_DASHBOARD;
    }
}

void printSerialLog() {
    const SensorData& sensor = ectSensor.getData();
    const SystemSettings& settings = storage.getSettings();

    Serial.print(F("[CR-V ECT] Suhu: "));
    Serial.print(sensor.temperature, 1);
    Serial.print(F(" C (Raw: "));
    Serial.print(sensor.rawTemperature, 1);
    Serial.print(F(" C) | Volt: "));
    Serial.print(sensor.rawVoltage, 2);
    Serial.print(F("V (ADC: "));
    Serial.print(sensor.rawAdc);
    Serial.print(F(") | Offset Aktif: "));
    if (sensor.currentOffset >= 0) Serial.print(F("+"));
    Serial.print(sensor.currentOffset, 1);
    Serial.print(F("C [O1:"));
    Serial.print((float)settings.tempOffset1 / 10.0f, 1);
    Serial.print(F(" O2:"));
    Serial.print((float)settings.tempOffset2 / 10.0f, 1);
    Serial.print(F(" O3:"));
    Serial.print((float)settings.tempOffset3 / 10.0f, 1);
    Serial.print(F("] | Fan: "));
    Serial.print(relayController.isFanOn() ? F("ON") : F("OFF"));
    if (!sensor.isValid) {
        Serial.print(F(" | ! FAIL-SAFE ACTIVE !"));
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n========================================"));
    Serial.println(F("CR-V Gen 3 Emergency Fan Controller"));
    Serial.println(F("Initializing system modules..."));
    Serial.println(F("========================================"));

    storage.begin();
    ectSensor.begin();
    buttonA.begin();
    buttonB.begin();
    relayController.begin();

    if (displayManager.begin()) {
        displayManager.showBootScreen();
    } else {
        Serial.println(F("[ERROR] OLED Display tidak terdeteksi di 0x3C!"));
    }

    Serial.println(F("System Ready."));
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. Tangani input tombol (dipanggil setiap iterasi untuk respon instan)
    handleMenuNavigation();

    // 2. Sampling sensor ECT (setiap 50ms)
    if (currentMillis - lastSensorTime >= INTERVAL_SENSOR_SAMPLE_MS) {
        lastSensorTime = currentMillis;
        const SystemSettings& settings = storage.getSettings();
        ectSensor.update(settings.tempOffset1, settings.tempOffset2, settings.tempOffset3);
    }

    // 3. Evaluasi logika Fan Relay (setiap 100ms)
    if (currentMillis - lastRelayTime >= INTERVAL_RELAY_UPDATE_MS) {
        lastRelayTime = currentMillis;
        relayController.update(ectSensor.getData(), storage.getSettings());
    }

    // 4. Perbarui Tampilan OLED (setiap 120ms - no flicker, no lag)
    if (currentMillis - lastDisplayTime >= INTERVAL_DISPLAY_UPDATE_MS) {
        lastDisplayTime = currentMillis;
        displayManager.update(
            currentMenu,
            ectSensor.getData(),
            storage.getSettings(),
            relayController.isFanOn()
        );
    }

    // 5. Telemetri Serial Debug (setiap 1000ms)
    if (currentMillis - lastSerialLogTime >= 1000) {
        lastSerialLogTime = currentMillis;
        printSerialLog();
    }
}
