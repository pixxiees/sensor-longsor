#include "read_soil.h"
#include <Arduino.h>

#define SOIL_PIN 2

// ============================================================
// HASIL KALIBRASI EMPIRIS SELESAI
// Udara bebas (Kering = 0%), Dicelup air (Saturasi Jenuh = 100%)
// ============================================================
const int ADC_DRY = 3400;
const int ADC_WET = 1190;

bool initSoil()
{
    pinMode(SOIL_PIN, INPUT);

    // Konfigurasi Attenuation agar pembacaan ADC ESP32-S3 linier di range 0-3.3V
    analogSetAttenuation(ADC_11db);
    analogReadResolution(12);

    Serial.println("[SOIL] Sensor Soil Moisture READY (GPIO 2)");

    return true;
}

// Fungsi membaca raw ADC tunggal
int readSoilADC()
{
    return analogRead(SOIL_PIN);
}

// Fungsi membaca persentase kelembapan dengan averaging 10 sampel (Bebas Noise)
float readSoilPercent()
{
    long sumADC = 0;
    for (int i = 0; i < 10; i++) {
        sumADC += analogRead(SOIL_PIN);
        delay(2); // Delay sangat singkat untuk sampel yang stabil
    }
    int avgADC = sumADC / 10;

    // Konversi nilai ADC ke Persentase (0 - 100%)
    float moisture = map(avgADC,
                         ADC_DRY,
                         ADC_WET,
                         0,
                         100);

    // Batasi nilai agar berada di rentang presisi 0.0% - 100.0%
    moisture = constrain(moisture, 0.0f, 100.0f);

    return moisture;
}