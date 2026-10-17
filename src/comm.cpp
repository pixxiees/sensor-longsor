#include "comm.h"

#include <SPI.h>
#include <RadioLib.h>

// ============================================================
// Heltec Wireless Stick Lite V3
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

    int state = radio.begin(915.0);

    if(state != RADIOLIB_ERR_NONE)
    {
        Serial.print("FAILED (");
        Serial.print(state);
        Serial.println(")");

        return false;
    }

    // --- PERBAIKAN 1: Wajib untuk Heltec V3 ---
    // Mengaktifkan saklar fisik antena agar bisa mengirim (TX) dan menerima (RX)
    radio.setDio2AsRfSwitch(true);

    // --- PERBAIKAN 2: Cegah Saturasi Sinyal ---
    // Turunkan daya pancar dari 22 menjadi 10 saat pengujian di atas meja
    // (Bisa dikembalikan ke 22 nanti saat dipasang di lapangan)
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

    Serial.print("FAILED (");
    Serial.print(state);
    Serial.println(")");

    return false;
}