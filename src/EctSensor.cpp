#include "EctSensor.h"

struct EctPoint {
    uint16_t adc;
    float temp;
};

// Tabel kalibrasi kurva NTC ECT Honda CR-V (ADC vs Suhu °C)
// Nilai ADC diurutkan dari tinggi ke rendah (tegangan turun seiring naiknya suhu NTC)
static const EctPoint HONDA_ECT_LUT[] = {
    { 950, -20.0f },
    { 880, -10.0f },
    { 845,   0.0f },
    { 785,  10.0f },
    { 715,  20.0f },
    { 630,  30.0f },
    { 510,  40.0f },
    { 415,  50.0f },
    { 335,  60.0f },
    { 265,  70.0f },
    { 230,  75.0f },
    { 205,  80.0f },
    { 180,  85.0f },
    { 160,  90.0f },
    { 142,  95.0f },
    { 126, 100.0f },
    { 112, 105.0f },
    { 100, 110.0f },
    {  88, 115.0f },
    {  78, 120.0f },
    {  60, 130.0f }
};

static const uint8_t LUT_SIZE = sizeof(HONDA_ECT_LUT) / sizeof(HONDA_ECT_LUT[0]);

EctSensor::EctSensor(uint8_t pin) 
    : sensorPin(pin), filteredAdc(0.0f), isFirstRead(true) {
    data.temperature = 0.0f;
    data.rawVoltage = 0.0f;
    data.rawAdc = 0;
    data.isValid = false;
    data.isShortCircuit = false;
    data.isOpenCircuit = false;
}

void EctSensor::begin() {
    pinMode(sensorPin, INPUT);
    // Lakukan pembacaan awal untuk inisialisasi ADC
    analogRead(sensorPin);
}

void EctSensor::checkSafetyThresholds(uint16_t adcVal) {
    if (adcVal < SENSOR_ADC_MIN_SAFE) {
        // Korslet ke Ground (Tegangan drop < ~0.12V)
        data.isValid = false;
        data.isShortCircuit = true;
        data.isOpenCircuit = false;
    } else if (adcVal > SENSOR_ADC_MAX_SAFE) {
        // Kabel putus / open circuit (Tegangan mendekati 5V)
        data.isValid = false;
        data.isShortCircuit = false;
        data.isOpenCircuit = true;
    } else {
        data.isValid = true;
        data.isShortCircuit = false;
        data.isOpenCircuit = false;
    }
}

float EctSensor::adcToTemperature(float adcVal) {
    // Batas bawah kurva
    if (adcVal >= HONDA_ECT_LUT[0].adc) {
        return HONDA_ECT_LUT[0].temp;
    }
    // Batas atas kurva
    if (adcVal <= HONDA_ECT_LUT[LUT_SIZE - 1].adc) {
        return HONDA_ECT_LUT[LUT_SIZE - 1].temp;
    }

    // Interpolasi linier antar titik dalam tabel
    for (uint8_t i = 0; i < LUT_SIZE - 1; ++i) {
        if (adcVal <= HONDA_ECT_LUT[i].adc && adcVal >= HONDA_ECT_LUT[i + 1].adc) {
            float adcRange = (float)(HONDA_ECT_LUT[i].adc - HONDA_ECT_LUT[i + 1].adc);
            float tempRange = HONDA_ECT_LUT[i + 1].temp - HONDA_ECT_LUT[i].temp;
            float factor = (float)(HONDA_ECT_LUT[i].adc - adcVal) / adcRange;
            return HONDA_ECT_LUT[i].temp + (factor * tempRange);
        }
    }

    return 25.0f; // Default fallback
}

void EctSensor::update(int8_t tempOffset) {
    // Multisample 4x untuk meredam noise frekuensi tinggi
    uint32_t adcSum = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        adcSum += analogRead(sensorPin);
    }
    uint16_t currentAdc = adcSum / 4;
    data.rawAdc = currentAdc;
    data.rawVoltage = (currentAdc * 5.0f) / 1023.0f;

    // Evaluasi fail-safe hardware
    checkSafetyThresholds(currentAdc);

    if (isFirstRead) {
        filteredAdc = (float)currentAdc;
        isFirstRead = false;
    } else {
        // Exponential Moving Average (EMA) filter: 85% lama + 15% baru
        // Menjamin pembacaan sangat stabil tanpa fluktuasi mendadak
        filteredAdc = (filteredAdc * 0.85f) + ((float)currentAdc * 0.15f);
    }

    if (data.isValid) {
        float calculatedTemp = adcToTemperature(filteredAdc);
        data.temperature = calculatedTemp + (float)tempOffset;
    } else {
        // Jika sensor error, set suhu ke nilai darurat tinggi (120°C)
        data.temperature = 120.0f;
    }
}

const SensorData& EctSensor::getData() const {
    return data;
}
