#include "read_sht.h"

// Objek SHT31 (Alamat I2C default modul ungu SHT30/SHT31 adalah 0x44)
Adafruit_SHT31 sht31 = Adafruit_SHT31();

bool initSHT() {
    // Memakai bus I2C yang sama dengan MPU6050
    if (!sht31.begin(0x44)) {   
        Serial.println("[SHT31] SHT30/SHT31 NOT FOUND at 0x44");
        return false;
    }
    Serial.println("[SHT31] SHT30/SHT31 READY");
    return true;
}

SHTData readSHT() {
    SHTData data;
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        data.temperature = t;
        data.humidity = h;
        data.isValid = true;
    } else {
        Serial.println("[SHT31] Failed to read temperature/humidity!");
        data.temperature = 0.0;
        data.humidity = 0.0;
        data.isValid = false;
    }
    return data;
}