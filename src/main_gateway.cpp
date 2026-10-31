#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <fuzzy_logic.h>
#include "server_http.h"

// ============================================================
//  Pin mapping Heltec WiFi LoRa 32 V3 (ESP32-S3) untuk Gateway
//  SPI LoRa (FSPI) : SCK=9, MISO=11, MOSI=10
//  SX1262          : NSS=8, DIO1=14, RESET=12, BUSY=13
// ============================================================
#define DIO1_PIN 14   // DIO1 = HIGH saat paket diterima

// --- PIN INDIKATOR RELAY GATEWAY (GPIO 2 & 3) ---
#define RELAY_KUNING 2   // Channel 1: Lampu Kuning (Status AWAS)
#define RELAY_MERAH  3   // Channel 2: Lampu Merah + Buzzer (Status BAHAYA)

#define RELAY_ON  HIGH
#define RELAY_OFF LOW

SPIClass LoRaSPI(FSPI);
SX1262 radio = new Module(8, DIO1_PIN, 12, 13, LoRaSPI);

unsigned long lastPrintTime = 0;
int waitDots = 0;

// Fungsi Mengatur Lampu Relay Peringatan
void setWarningSystem(int levelStatus) {
    if (levelStatus == 1) {
        // --- AMAN ---
        digitalWrite(RELAY_KUNING, RELAY_OFF);
        digitalWrite(RELAY_MERAH, RELAY_OFF);
        Serial.println("[STATUS] AMAN -> Semua Lampu & Alarm Mati");

    } else if (levelStatus == 2) {
        // --- AWAS ---
        digitalWrite(RELAY_KUNING, RELAY_ON);
        digitalWrite(RELAY_MERAH, RELAY_OFF);
        Serial.println("[STATUS] AWAS -> Lampu Kuning Menyala");

    } else if (levelStatus == 3) {
        // --- BAHAYA ---
        digitalWrite(RELAY_KUNING, RELAY_ON);
        digitalWrite(RELAY_MERAH, RELAY_ON); // Lampu Merah + Buzzer + Lampu Kuning Menyala
        Serial.println("[STATUS] BAHAYA -> Lampu Kuning & (Lampu Merah + Buzzer) Menyala!");
    }
}

void setup() {
    // 1. SET KE LOW TERLEBIH DAHULU UNTUK MENCEGAH RELAY NYALA SAAT BOOTING
    digitalWrite(RELAY_KUNING, RELAY_OFF);
    digitalWrite(RELAY_MERAH,  RELAY_OFF);

    pinMode(RELAY_KUNING, OUTPUT);
    pinMode(RELAY_MERAH,  OUTPUT);

    // Paksa mati kembali ke kondisi AMAN
    setWarningSystem(1);

    Serial.begin(115200);
    delay(1000);

    Serial.println("=================================");
    Serial.println("  Heltec V3 Lite Gateway (RX)   ");
    Serial.println("=================================");
    Serial.println("[SYSTEM] Fixing GPIO 2 & 3... OK (Relay Standby MATI)");

    // Daya VEXT untuk Chip LoRa SX1262
    pinMode(36, OUTPUT);
    digitalWrite(36, LOW); // LOW = Menyalakan VEXT
    delay(100);

    // Cegah pin interupsi DIO1 floating saat boot
    pinMode(DIO1_PIN, INPUT_PULLDOWN);

    // Inisialisasi WiFi
    setupWiFi();

    // Sesi baru database
    startNewSession();

    // Inisialisasi SPI LoRa
    LoRaSPI.begin(9, 11, 10, 8);

    // Inisialisasi Radio SX1262
    int state = radio.begin(915.0);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("[LoRa] Inisialisasi SX1262 SUCCESS");

        radio.setSpreadingFactor(11);
        radio.setBandwidth(62.5);
        radio.setCodingRate(7);
        radio.setSyncWord(0x12);
        radio.setDio2AsRfSwitch(true);

    } else {
        Serial.printf("[LoRa] Inisialisasi FAILED, code = %d\n", state);
        while (true) { delay(500); }
    }

    // Inisialisasi Fuzzy Logic
    setupFuzzy();
    Serial.println("[Fuzzy] 27 Aturan berhasil dimuat.");

    // Mulai mode mendengarkan (receive)
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

    if (digitalRead(DIO1_PIN) == HIGH) {
        String received = "";
        int state = radio.readData(received);

        if (state == RADIOLIB_ERR_NONE) {
            Serial.println();
            Serial.println("========== PAKET DITERIMA ==========");
            Serial.print("[RX] Pesan   : ");
            Serial.println(received);

            // Parsing data dari Node
            char node_id[20]; 
            int counter, rain;
            float roll, pitch, soil, temp, vbat_raw;
            float rssi = radio.getRSSI();
            float snr = radio.getSNR();

            int parsed = sscanf(received.c_str(), "%19[^,],%d,%f,%f,%d,%f,%f,%f", 
                                node_id, &counter, &roll, &pitch, &rain, &soil, &temp, &vbat_raw);

            if (parsed == 8) {
                // Kalibrasi tegangan baterai
                float vbat_calibrated = vbat_raw * 1.799f;

                Serial.println("[PARSING] Succeed!");

                // --- EKSEKUSI FUZZY LOGIC ---
                float pergerakan_max = max(abs(roll), abs(pitch));

                // Panggil fungsi fuzzy_logic.cpp
                int status_fuzzy = hitungFuzzy((float)rain, soil, pergerakan_max);

                // Atur Relay sesuai hasil Fuzzy Logic
                setWarningSystem(status_fuzzy);

                // Konversi nilai integer status_fuzzy ke Teks String
                String strFuzzy = "AMAN";
                if (status_fuzzy == 2) {
                    strFuzzy = "AWAS";
                } else if (status_fuzzy == 3) {
                    strFuzzy = "BAHAYA";
                }

                // Kirim data lengkap beserta fuzzy_status ke PythonAnywhere Server
                sendDataToServer(String(node_id), counter, roll, pitch, rain, soil, temp, vbat_calibrated, rssi, snr, strFuzzy);
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

        radio.startReceive();
    }
}