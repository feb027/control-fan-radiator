#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// PIN DEFINITIONS (ARDUINO NANO)
// ==========================================
#define PIN_ECT_SENSOR      A0    // Input pembacaan sinyal ECT bawaan mobil
#define PIN_BUTTON_A        2     // Tombol A (Set / Nilai +) - Aktif LOW (INPUT_PULLUP)
#define PIN_BUTTON_B        3     // Tombol B (Next / Nilai -) - Aktif LOW (INPUT_PULLUP)
#define PIN_RELAY_FAN       7     // Output ke Modul Relay Fan Radiator

// I2C OLED 0.91" (128x32) menggunakan pin I2C default ATmega328P:
// SDA -> A4
// SCL -> A5

// ==========================================
// RELAY SETTINGS
// ==========================================
// Modul relay Arduino aktif LOW (LOW = Relay ON / Kipas Nyala, HIGH = Relay OFF / Kipas Mati).
#define RELAY_ACTIVE_LOW    true

// ==========================================
// DEFAULT TEMPERATURE SETPOINTS (°C)
// ==========================================
#define DEFAULT_TEMP_UPPER       96   // Suhu batas atas (Fan ON)
#define DEFAULT_TEMP_LOWER       90   // Suhu batas bawah (Fan OFF)

// ==========================================
// 3-ZONE CALIBRATION OFFSETS (TENTHS OF °C)
// ==========================================
// Disimpan dalam satuan 0.1°C (-100 = -10.0°C)
#define DEFAULT_TEMP_OFFSET_1    -100  // Offset Zona 1 (Acuan ~50°C / Rentang 40-60°C)
#define DEFAULT_TEMP_OFFSET_2    -100  // Offset Zona 2 (Acuan ~70°C / Rentang 60-80°C)
#define DEFAULT_TEMP_OFFSET_3    -100  // Offset Zona 3 (Acuan ~90°C / Rentang >80°C)

#define MIN_ALLOWABLE_OFFSET     -250  // Batas minimum kalibrasi (-25.0°C)
#define MAX_ALLOWABLE_OFFSET      150  // Batas maksimum kalibrasi (+15.0°C)
#define OFFSET_STEP_TENTHS          1  // Resolusi per klik: 0.1°C

#define MIN_ALLOWABLE_TEMP_LOWER 70   // Batas minimum settingan Fan OFF
#define MAX_ALLOWABLE_TEMP_UPPER 110  // Batas maksimum settingan Fan ON
#define MIN_HYSTERESIS_GAP        3   // Selisih minimal antara Fan ON dan Fan OFF (°C)

// ==========================================
// SENSOR SAFETY & FAIL-SAFE THRESHOLDS
// ==========================================
// Jika kabel putus atau korslet ke ground, ADC akan di luar batas wajar.
// Dalam kondisi darurat, Fan akan dipaksa ON secara otomatis!
#define SENSOR_ADC_MIN_SAFE      25   // ~0.12V (indikasi korslet ke Ground)
#define SENSOR_ADC_MAX_SAFE     995   // ~4.85V (indikasi kabel putus / open circuit)

// ==========================================
// TIMING INTERVALS (NON-BLOCKING millis())
// ==========================================
#define INTERVAL_SENSOR_SAMPLE_MS   50    // Pembacaan ADC & filter EMA
#define INTERVAL_DISPLAY_UPDATE_MS  120   // Refresh layar OLED (smooth & bebas flicker)
#define INTERVAL_RELAY_UPDATE_MS    100   // Evaluasi logika suhu vs relay
#define INTERVAL_MENU_TIMEOUT_MS    10000 // Otomatis kembali ke dashboard setelah 10 detik

#define BUTTON_DEBOUNCE_MS          35    // Debounce tombol
#define BUTTON_LONG_PRESS_MS        1000  // Durasi tahan tombol (1 detik)

// ==========================================
// OLED SPECIFICATIONS
// ==========================================
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       32
#define OLED_RESET_PIN      -1
#define OLED_I2C_ADDRESS    0x3C          // Alamat I2C umum OLED 0.91" (0x3C atau 0x3D)
#define OLED_ROTATION       2             // 0: Normal, 2: Dibalik 180 derajat (Upside-down)

// ==========================================
// EEPROM STORAGE CONFIG
// ==========================================
#define EEPROM_MAGIC_KEY    0x5433        // 'T3' (Temp 3-Zone Controller) Magic Word
#define EEPROM_START_ADDR   0

#endif // CONFIG_H
