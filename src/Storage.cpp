#include "Storage.h"
#include <EEPROM.h>

Storage::Storage() {
    resetToDefaults();
}

void Storage::begin() {
    SystemSettings storedData;
    EEPROM.get(EEPROM_START_ADDR, storedData);

    if (isValid(storedData)) {
        settings = storedData;
    } else {
        // Jika data EEPROM belum pernah ditulis atau korup, gunakan setelan default
        resetToDefaults();
        saveSettings();
    }
}

SystemSettings& Storage::getSettings() {
    return settings;
}

uint8_t Storage::calculateChecksum(const SystemSettings& data) {
    uint8_t sum = 0;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&data);
    // Hitung seluruh byte kecuali field checksum terakhir
    for (size_t i = 0; i < sizeof(SystemSettings) - sizeof(uint8_t); ++i) {
        sum ^= ptr[i];
    }
    return sum;
}

bool Storage::isValid(const SystemSettings& data) {
    if (data.magicKey != EEPROM_MAGIC_KEY) return false;
    if (calculateChecksum(data) != data.checksum) return false;
    if (data.tempUpper <= data.tempLower) return false;
    if (data.tempLower < MIN_ALLOWABLE_TEMP_LOWER) return false;
    if (data.tempUpper > MAX_ALLOWABLE_TEMP_UPPER) return false;
    if (data.tempOffset < -25 || data.tempOffset > 25) return false;
    return true;
}

bool Storage::saveSettings() {
    // Validasi konsistensi histeresis sebelum menyimpan
    if (settings.tempUpper <= settings.tempLower + MIN_HYSTERESIS_GAP) {
        settings.tempUpper = settings.tempLower + MIN_HYSTERESIS_GAP;
    }
    settings.magicKey = EEPROM_MAGIC_KEY;
    settings.checksum = calculateChecksum(settings);

    // EEPROM.put menggunakan EEPROM.update di internalnya sehingga hemat siklus tulis
    EEPROM.put(EEPROM_START_ADDR, settings);
    return true;
}

void Storage::resetToDefaults() {
    settings.magicKey = EEPROM_MAGIC_KEY;
    settings.tempUpper = DEFAULT_TEMP_UPPER;
    settings.tempLower = DEFAULT_TEMP_LOWER;
    settings.tempOffset = DEFAULT_TEMP_OFFSET;
    settings.checksum = calculateChecksum(settings);
}
