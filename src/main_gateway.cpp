#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <fuzzy_logic.h>
#include "server_http.h" // Tambahkan library modular yang baru dibuat

// ============================================================
//  Pin mapping Heltec WiFi LoRa 32 V3 (ESP32-S3)
//  SPI LoRa (FSPI) : SCK=9, MISO=11, MOSI=10
//  SX1262          : NSS=8, DIO1=14, RESET=12, BUSY=13
// ============================================================
#define DIO1_PIN 14   // DIO1 = HIGH saat paket diterima

SPIClass LoRaSPI(FSPI);
SX1262 radio = new Module(8, DIO1_PIN, 12, 13, LoRaSPI);

unsigned long lastPrintTime = 0;
int waitDots = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=================================");
    Serial.println("  Heltec WiFi LoRa 32 V3 (RX)  ");
    Serial.println("    LoRa RX - Node Penerima     ");
    Serial.println("=================================");

    // Inisialisasi WiFi Modular
    setupWiFi();

    LoRaSPI.begin(9, 11, 10, 8);

    int state = radio.begin(915.0); // Sesuaikan dengan frekuensi TX
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[LoRa] Inisialisasi SUCCESS");
        
        // --- PERBAIKAN: WAJIB UNTUK HELTEC V3 ---
        // Menyalakan saklar fisik antena agar bisa menerima sinyal (RX)
        radio.setDio2AsRfSwitch(true);
        
    } else {
        Serial.printf("[LoRa] FAILED, code = %d\n", state);
        while (true) { delay(500); }
    }

    // Inisialisasi Fuzzy dari fuzzy_logic.cpp
    setupFuzzy();
    Serial.println("[Fuzzy] 27 Aturan berhasil dimuat.");

    radio.startReceive();
}

void loop() {
    if (millis() - lastPrintTime >= 3000) {
        lastPrintTime = millis();
        waitDots = (waitDots % 3) + 1;
        Serial.print("[RX] Menunggu paket");
        for (int i = 0; i < waitDots; i++) Serial.print(".");
        Serial.println();
    }

    // Cek DIO1 — HIGH berarti SX1262 sudah terima paket
    if (digitalRead(DIO1_PIN) == HIGH) {
        String received = "";
        int state = radio.readData(received);

        if (state == RADIOLIB_ERR_NONE) {
            Serial.println();
            Serial.println("========== PAKET DITERIMA ==========");
            Serial.print("[RX] Pesan   : ");
            Serial.println(received);
            Serial.print("[RX] RSSI    : ");
            Serial.print(radio.getRSSI());
            Serial.println(" dBm");
            Serial.print("[RX] SNR     : ");
            Serial.print(radio.getSNR());
            Serial.println(" dB");
            
            // ==========================================
            // PARSING DATA
            // ==========================================
            int counter;
            float roll, pitch, rain, soil;

            // dummy rain
            rain = random(0, 601) / 10.0;

            int parsed = sscanf(received.c_str(), "%d,%f,%f,%f", 
                                &counter, &roll, &pitch, &soil);

            if (parsed == 4) {
                Serial.println("[PARSING] Succeed!");
                Serial.printf("Roll: %.2f\r\n", roll);
                Serial.printf("Pitch: %.2f\r\n", pitch);
                Serial.printf("Rain: %.2f\r\n", rain);
                Serial.printf("Soil Moisture: %.2f %%\r\n", soil);
                
                // Eksekusi fuzzy (sebagai contoh)
                float kemiringan = max(abs(pitch), abs(roll));
                
                // Kirim data ke web server menggunakan fungsi modular
                sendDataToServer(counter, roll, pitch, rain, soil);
                
            } else {
                Serial.println("[PARSING] Failed! Format data tidak sesuai (bukan 5 data).");
            }

            Serial.println("====================================");
            lastPrintTime = millis();
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            Serial.println("[RX] CRC Error - packet corrupt!");
        } else {
            Serial.print("[RX] readData error, code = ");
            Serial.println(state);
        }

        // Restart receive untuk paket berikutnya
        radio.startReceive();
    }
}