#ifndef ECT_SENSOR_H
#define ECT_SENSOR_H

#include "Config.h"
#include "Types.h"

class EctSensor {
public:
    EctSensor(uint8_t pin);
    void begin();
    void update(int16_t offset1, int16_t offset2, int16_t offset3);
    const SensorData& getData() const;

private:
    uint8_t sensorPin;
    SensorData data;
    float filteredAdc;
    bool isFirstRead;

    float adcToTemperature(float adcVal);
    float calculateInterpolatedOffset(float rawTemp, float o1, float o2, float o3);
    void checkSafetyThresholds(uint16_t adcVal);
};

#endif // ECT_SENSOR_H
