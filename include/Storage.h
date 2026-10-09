#ifndef STORAGE_H
#define STORAGE_H

#include "Types.h"
#include "Config.h"

class Storage {
public:
    Storage();
    void begin();
    SystemSettings& getSettings();
    bool saveSettings();
    void resetToDefaults();

private:
    SystemSettings settings;
    uint8_t calculateChecksum(const SystemSettings& data);
    bool isValid(const SystemSettings& data);
};

#endif // STORAGE_H
