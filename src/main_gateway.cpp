#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <fuzzy_logic.h>
#include "server_http.h" // Library modular untuk kirim data ke server/HTTP

// ============================================================
//  Pin mapping Heltec WiFi LoRa 32 V3 (ESP32-S3) untuk Gateway
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
    Serial.println("    LoRa RX - Node Gateway      ");
    Serial.println("=================================");

    pinMode(36, OUTPUT);
    digitalWrite(36, LOW); // LOW = Menyalakan VEXT (Memberi daya ke SX1262)
    delay(100);            // Beri jeda 100ms agar tegangan LoRa stabil

    Serial.begin(115200);
    delay(2000);

    Serial.println("=================================");
    Serial.println("  Heltec V3 Lite Gateway (RX)   ");
    Serial.println("=================================");

    // Inisialisasi WiFi Modular
    setupWiFi();

    // Inisialisasi SPI khusus LoRa
    LoRaSPI.begin(9, 11, 10, 8);

    // Inisialisasi Radio pada Frekuensi 915.0 MHz
    int state = radio.begin(915.0);
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[LoRa] Inisialisasi SX1262 SUCCESS");
        
        // Parameter radio SAMA PERSIS dengan Node TX
        radio.setSpreadingFactor(11);
        radio.setBandwidth(62.5);
        radio.setCodingRate(7);
        radio.setSyncWord(0x12);
      
        radio.setDio2AsRfSwitch(true);
        
    } else {
        Serial.printf("[LoRa] Inisialisasi FAILED, code = %d\n", state);
        while (true) { delay(500); }
    }

    // Inisialisasi Fuzzy dari fuzzy_logic.cpp
    setupFuzzy();
    Serial.println("[Fuzzy] 27 Aturan berhasil dimuat.");

    // Mulai mode mendengarkan (receive)
    radio.startReceive();
}

void loop() {
    // Indikator visual di Serial Monitor bahwa gateway aktif menunggu
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
            // PARSING DATA DARI NODE
            // ==========================================
            char node_id[20]; 
            int counter, rain;
            float roll, pitch, soil, temp, vbat;
            float rssi = radio.getRSSI();
            float snr = radio.getSNR();

            int parsed = sscanf(received.c_str(), "%19[^,],%d,%f,%f,%d,%f,%f,%f", 
                                node_id, &counter, &roll, &pitch, &rain, &soil, &temp, &vbat);

            if (parsed == 8) {
                Serial.println("[PARSING] Succeed!");
                
                // Kirim data terurai ke server lewat HTTP
                sendDataToServer(String(node_id), counter, roll, pitch, rain, soil, temp, vbat, rssi, snr);
            } else {
                Serial.println("[PARSING] Failed! Format data tidak sesuai.");
            }

            Serial.println("====================================");
            lastPrintTime = millis();
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            Serial.println("[RX] CRC Error - packet corrupt!");
        } else {
            Serial.print("[RX] readData error, code = ");
            Serial.println(state);
        }

        // Mulai ulang mode penerimaan untuk paket berikutnya
        radio.startReceive();
    }
}