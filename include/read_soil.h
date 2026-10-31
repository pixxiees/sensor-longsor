#ifndef READ_SOIL_H
#define READ_SOIL_H

#include <Arduino.h>

bool initSoil();
int readSoilADC();
float readSoilPercent();

#endif // READ_SOIL_H