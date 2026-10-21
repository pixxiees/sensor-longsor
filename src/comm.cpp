#include "comm.h"
#include <SPI.h>
#include <RadioLib.h>

// ============================================================
// Konfigurasi Pin Heltec WiFi LoRa 32 V3 / Wireless Stick Lite V3
// ============================================================

SPIClass LoRaSPI(FSPI);

SX1262 radio = new Module(
    8,      // NSS
    14,     // DIO1
    12,     // RESET
    13,     // BUSY
    LoRaSPI
);

bool initLoRa()
{
    LoRaSPI.begin(
        9,      // SCK
        11,     // MISO
        10,     // MOSI
        8       // SS
    );

    Serial.print("[LoRa] Initializing... ");

    // 1. Inisialisasi Frekuensi (915.0 MHz untuk Indonesia)
    int state = radio.begin(915.0);

    if(state != RADIOLIB_ERR_NONE)
    {
        Serial.print("FAILED (Kode Error: ");
        Serial.print(state);
        Serial.println(")");
        return false;
    }

    // ========================================================
    // 2. SINKRONISASI PARAMETER LORA (Wajib sama di Node & Gateway)
    // ========================================================
    
    // Spreading Factor 11 untuk penetrasi halangan (box 3D/dinding)
    radio.setSpreadingFactor(11);
    
    // Bandwidth 62.5 kHz (Di RadioLib ditulis langsung dalam kHz)
    radio.setBandwidth(62.5);
    
    // Coding Rate 7 (Lebih tahan terhadap error/noise)
    radio.setCodingRate(7);

    // Sync Word 0x12 (Standar private network LoRa)
    radio.setSyncWord(0x12);
    
    // ========================================================

    // 3. Konfigurasi Khusus Hardware Heltec V3
    // Wajib diaktifkan agar antena bisa berpindah mode RX dan TX
    radio.setDio2AsRfSwitch(true);

    // Turunkan daya pancar ke 15 dBm untuk mencegah saturasi sinyal
    // saat kedua alat diuji dalam jarak dekat di atas meja.
    radio.setOutputPower(15);

    Serial.println("SUCCESS");
    return true;
}

bool sendLoRa(String &message)
{
    Serial.print("[TX] ");
    Serial.print(message);
    Serial.print(" ... ");

    int state = radio.transmit(message);

    if(state == RADIOLIB_ERR_NONE)
    {
        Serial.println("OK");
        return true;
    }

    Serial.print("FAILED (Kode Error: ");
    Serial.print(state);
    Serial.println(")");

    return false;
}