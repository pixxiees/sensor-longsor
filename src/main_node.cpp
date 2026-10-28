#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include "comm.h"
#include "read_imu.h"
#include "read_soil.h"
#include "read_rain.h"

// --- KONFIGURASI DEEP SLEEP & INTERUPSI GPIO 4 ---
#define TIME_TO_SLEEP  180         // Deep sleep timer 3 menit
#define uS_TO_S_FACTOR 1000000ULL  
#define ANOMALY_PIN    GPIO_NUM_4  // Menggunakan GPIO 4

RTC_DATA_ATTR uint32_t counter = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\r\n=================================");
    Serial.println(" HELTEC SENSOR NODE (GPIO 4 INT)");
    Serial.println("=================================");

    // Cek Alasan Bangun
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("[NODE] !!! DETEKSI LONGSOR !!! Bangun Seketika via INT MPU6050!");
    } else {
        Serial.println("[NODE] Bangun Rutin / Booting Awal");
    }

    // Inisialisasi Hardware
    if(!initLoRa())  { Serial.println("[ERROR] LoRa Init Failed!"); }
    if(!initIMU())   { Serial.println("[ERROR] IMU Init Failed!"); }
    if(!initSoil())  { Serial.println("[ERROR] Soil Init Failed!"); }
    setupRain();

    counter++;

    // Baca Sensor
    float soil = readSoilPercent();
    IMUData imu = readIMU();
    int jumlahTip = getRainTips();
    float vbat = (analogRead(1) / 4095.0) * 3.3 * 3.2;

    // Print Debug
    Serial.println("--------------------------------");
    Serial.printf("Counter : %d\r\n", counter);
    Serial.printf("SOIL    : %.1f %%\r\n", soil);
    Serial.printf("Pitch   : %.2f\r\n", imu.pitch);
    Serial.printf("Roll    : %.2f\r\n", imu.roll);
    Serial.printf("Rain    : %d\r\n", jumlahTip);
    Serial.printf("Baterai : %.2f V\r\n", vbat);

    // Rakit & Kirim Payload
    String payload = 
        String("NODE_1") + "," +                
        String(counter) + "," +                
        String(imu.roll, 2) + "," +            
        String(imu.pitch, 2) + "," +           
        String(jumlahTip) + "," +              
        String(soil, 1) + "," +                
        String(imu.temperature, 2) + "," +     
        String(vbat, 2);                       

    Serial.print("[LoRa] Kirim Payload: ");
    Serial.println(payload);
    sendLoRa(payload);

    // 1. Konfigurasi RTC Internal Pull-Down Khusus GPIO 4
    rtc_gpio_init(ANOMALY_PIN);
    rtc_gpio_set_direction(ANOMALY_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_dis(ANOMALY_PIN);
    rtc_gpio_pulldown_en(ANOMALY_PIN); // Mengunci Pull-Down internal ke GND saat sleep

    // 2. Clear status interupsi MPU6050
    readIMU();
    delay(100);

    // 3. Aktifkan Dual Wakeup (Timer 3 Menit + Interupsi HIGH GPIO 4)
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(ANOMALY_PIN, 1);

    Serial.println("[NODE] Masuk Deep Sleep (Timer + INT GPIO 4)...");
    Serial.flush();
    esp_deep_sleep_start();
}

void loop() {
}