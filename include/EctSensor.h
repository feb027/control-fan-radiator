#ifndef ECT_SENSOR_H
#define ECT_SENSOR_H

#include "Config.h"
#include "Types.h"

class EctSensor {
public:
    EctSensor(uint8_t pin);
    void begin();
    void update(int8_t tempOffset);
    const SensorData& getData() const;

private:
    uint8_t sensorPin;
    SensorData data;
    float filteredAdc;
    bool isFirstRead;

    float adcToTemperature(float adcVal);
    void checkSafetyThresholds(uint16_t adcVal);
};

#endif // ECT_SENSOR_H
