#include <Arduino.h>
#include "comm.h"
#include "read_imu.h"
#include "read_soil.h"
#include "read_rain.h"

uint32_t counter = 0;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("=================================");
    Serial.println(" HELTEC SENSOR NODE (ACTIVE)     ");
    Serial.println("=================================");

    if(!initLoRa())
    {
        while(true)
        {
            delay(1000);
        }
    }

    if(!initIMU())
    {
        while(true)
        {
            delay(1000);
        }
    }

    if(!initSoil())
    {
        while(true)
        {
            delay(1000);
        }
    }

    setupRain();
    Serial.println("[Setup] Rain Gauge Interrupt ready.");

    Serial.println();
    Serial.println("System Ready");
    Serial.println();
}

void loop()
{
    Serial.println("--------------------------------");
    counter++;

    // 1. Baca Sensor-sensor Fisik
    float soil = readSoilPercent();
    IMUData imu = readIMU();
    int jumlahTip = getRainTips();

    // 2. Baca Tegangan Baterai (Pin GPIO 1 / ADC)
    float vbat = (analogRead(1) / 4095.0) * 3.3 * 3.2;

    // 3. Print Debug ke Serial Monitor
    Serial.print("SOIL   : "); Serial.print(soil); Serial.println(" %");
    Serial.print("Pitch  : "); Serial.println(imu.pitch);
    Serial.print("Roll   : "); Serial.println(imu.roll);
    Serial.print("TEMP   : "); Serial.println(imu.temperature);
    Serial.printf("Rain   : %d\r\n", jumlahTip);
    Serial.printf("Baterai: %.2f V\r\n", vbat);

    // 4. Rakit Payload (8 Komponen Data Sesuai Format Gateway)
    String payload = 
        String("NODE_1") + "," +                // 1. ID Node
        String(counter) + "," +                // 2. Counter
        String(imu.roll, 2) + "," +            // 3. Roll
        String(imu.pitch, 2) + "," +           // 4. Pitch
        String(jumlahTip) + "," +              // 5. Curah Hujan (Rain)
        String(soil, 1) + "," +                // 6. Soil Moisture
        String(imu.temperature, 2) + "," +     // 7. Suhu
        String(vbat, 2);                       // 8. Baterai

    // 5. Kirim via LoRa menggunakan fungsi sendLoRa dari comm.cpp
    sendLoRa(payload);

    // Jeda pengiriman (misal 10 detik atau 3 detik sesuai kebutuhan uji coba)
    delay(10000); 
}