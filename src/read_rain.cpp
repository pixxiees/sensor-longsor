#include "read_rain.h"
#include <Arduino.h>

#define RAIN_PIN 6

// ============================================================
// HASIL KALIBRASI EMPIRIS FISIK
// Corong Diameter 11 cm (A = 95.03 cm²), 50 mL = 5 tip (10 mL/tip)
// ============================================================
const float MM_PER_TIP = 1.0523f; 

// Atribut penahan data saat Deep Sleep
RTC_DATA_ATTR volatile int hitunganJungkit = 0;

volatile unsigned long waktuJungkitTerakhir = 0;
const unsigned long jedaBouncing = 50; // Disesuaikan dari 200 ms ke 50 ms

void IRAM_ATTR sensorHujanTerpicu() {
    unsigned long waktuSekarang = millis();
    // Logika debouncing
    if (waktuSekarang - waktuJungkitTerakhir > jedaBouncing) {
        hitunganJungkit++;
        waktuJungkitTerakhir = waktuSekarang;
    }
}

void setupRain() {
    pinMode(RAIN_PIN, INPUT_PULLUP);
    
    // Mode diganti ke CHANGE (Sesuai hasil uji kalibrasi mode CHANGE)
    attachInterrupt(digitalPinToInterrupt(RAIN_PIN), sensorHujanTerpicu, CHANGE);
    
    Serial.println("[RAIN] Tipping Bucket Interrupt READY (GPIO 6)");
}

// Fungsi mengembalikan total jumlah tip
int getRainTips() {
    noInterrupts();
    int tips = hitunganJungkit;
    interrupts();
    return tips;
}

// Fungsi mengembalikan total curah hujan dalam satuan mm
float getRainMM() {
    return getRainTips() * MM_PER_TIP;
}

// Fungsi reset konter hujan jika diperlukan
void resetRain() {
    noInterrupts();
    hitunganJungkit = 0;
    interrupts();
}