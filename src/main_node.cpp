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
    Serial.println(" HELTEC SENSOR NODE");
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

    float soil = readSoilPercent();

    Serial.print("SOIL : ");
    Serial.print(soil);
    Serial.println(" %");

    IMUData imu = readIMU();

    float vbat = (analogRead(1) / 4095.0) * 3.3 * 3.2;
    Serial.printf("Baterai: %.2f V\r\n", vbat);

    // Serial.print("AX : ");
    // Serial.println(imu.ax);

    // Serial.print("AY : ");
    // Serial.println(imu.ay);

    // Serial.print("AZ : ");
    // Serial.println(imu.az);

    // Serial.print("GX : ");
    // Serial.println(imu.gx);

    // Serial.print("GY : ");
    // Serial.println(imu.gy);

    // Serial.print("GZ : ");
    // Serial.println(imu.gz);

    Serial.print("Pitch : ");
    Serial.println(imu.pitch);

    Serial.print("Roll  : ");
    Serial.println(imu.roll);

    Serial.print("TEMP : ");
    Serial.println(imu.temperature);

    // int jumlahTip = getRainTips();
    // Serial.printf("Rain tipping: %d\r\n", jumlahTip);

    // 1. Baca curah hujan (tipping bucket)
    int jumlahTip = getRainTips();
    Serial.printf("Rain tipping: %d\r\n", jumlahTip);        

    // 2. Rakit payload (Pastikan jumlahnya 8 data)
    String payload = 
        String("NODE_1") + "," +               // 1. ID Node
        String(counter) + "," +                // 2. Counter
        String(imu.roll, 2) + "," +            // 3. Roll
        String(imu.pitch, 2) + "," +           // 4. Pitch
        String(jumlahTip) + "," +              // 5. Curah Hujan (Rain)
        String(soil, 1) + "," +                // 6. Soil Moisture
        String(imu.temperature, 2) + "," +     // 7. Suhu
        String(vbat, 2);                       // 8. Baterai

    // 3. Kirim via LoRa
    sendLoRa(payload);

    delay(3000);
}