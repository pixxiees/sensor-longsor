#ifndef READ_SHT_H
#define READ_SHT_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

struct SHTData {
    float temperature;
    float humidity;
    bool isValid;
};

bool initSHT();
SHTData readSHT();

#endif