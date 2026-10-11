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
unsigned long lastUserActivityTime = 0;
bool lastFanState = false;

// Pewaktu non-blocking
unsigned long lastSensorTime = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastRelayTime = 0;
unsigned long lastSerialLogTime = 0;

void handleMenuNavigation() {
    // Jika layar sedang mati/tidur (sleep mode)
    if (!displayManager.isDisplayOn()) {
        // Jika ada tombol ditekan, bangunkan layar dan abaikan aksi menu pada klik pertama ini
        if (digitalRead(PIN_BUTTON_A) == LOW || digitalRead(PIN_BUTTON_B) == LOW) {
            displayManager.setDisplayPower(true);
            lastUserActivityTime = millis();
            lastMenuInteraction = millis();
            buttonA.update();
            buttonB.update();
            return;
        }
        return;
    }

    ButtonEvent evtA = buttonA.update();
    ButtonEvent evtB = buttonB.update();

    if (evtA != BTN_EVENT_NONE || evtB != BTN_EVENT_NONE) {
        lastMenuInteraction = millis();
        lastUserActivityTime = millis();
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
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 1 (60-80°C)
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
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 2 (80-90°C)
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
                // Tekan tahan A untuk lanjut ke kalibrasi offset zona 3 (>90°C)
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
            } else if (evtA == BTN_EVENT_LONG_PRESS) {
                // Tekan tahan A untuk lanjut ke menu setting Auto-Sleep OLED
                currentMenu = MENU_SET_OLED_TIMEOUT;
            } else if (evtB == BTN_EVENT_LONG_PRESS) {
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;

        case MENU_SET_OLED_TIMEOUT:
            if (evtA == BTN_EVENT_SHORT_PRESS) {
                if (settings.oledTimeoutMin < MAX_OLED_TIMEOUT_MIN) {
                    settings.oledTimeoutMin++;
                }
            } else if (evtB == BTN_EVENT_SHORT_PRESS) {
                if (settings.oledTimeoutMin > MIN_OLED_TIMEOUT_MIN) {
                    settings.oledTimeoutMin--;
                }
            } else if (evtA == BTN_EVENT_LONG_PRESS || evtB == BTN_EVENT_LONG_PRESS) {
                // Simpan ke EEPROM dan kembali ke dashboard
                storage.saveSettings();
                currentMenu = MENU_DASHBOARD;
            }
            break;
    }

    // Auto-timeout Menu: Jika tidak ada tombol ditekan selama 10 detik,
    // simpan pengaturan dan otomatis kembali ke tampilan dashboard
    if (currentMenu != MENU_DASHBOARD && (millis() - lastMenuInteraction > INTERVAL_MENU_TIMEOUT_MS)) {
        storage.saveSettings();
        currentMenu = MENU_DASHBOARD;
    }

    // Auto-sleep Layar OLED:
    // Jika berada di dashboard dan waktu idle melebihi batas settingan (1 - 30 menit),
    // matikan layar. Layar tidak akan dimatikan jika ada sensor error atau kipas sedang hidup.
    if (currentMenu == MENU_DASHBOARD && displayManager.isDisplayOn()) {
        unsigned long sleepTimeoutMs = (unsigned long)settings.oledTimeoutMin * 60000UL;
        if (ectSensor.getData().isValid && !relayController.isFanOn()) {
            if (millis() - lastUserActivityTime >= sleepTimeoutMs) {
                displayManager.setDisplayPower(false);
            }
        }
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
    Serial.print(F(" | OLED: "));
    Serial.print(displayManager.isDisplayOn() ? F("ON") : F("SLEEP"));
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

    lastUserActivityTime = millis();
    lastFanState = false;

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

        // Bangunkan layar secara otomatis jika ada kondisi krusial:
        // - Sensor bermasalah (kabel putus atau korslet ke ground)
        // - Kipas radiator baru saja terpicu menyala (suhu mencapai batas atas)
        bool currentFanState = relayController.isFanOn();
        if (!ectSensor.getData().isValid || (currentFanState && !lastFanState)) {
            if (!displayManager.isDisplayOn()) {
                displayManager.setDisplayPower(true);
            }
            lastUserActivityTime = currentMillis;
        }
        lastFanState = currentFanState;
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
