#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include "comm.h"
#include "read_imu.h"
#include "read_soil.h"
#include "read_rain.h"

// --- KONFIGURASI DEEP SLEEP & INTERUPSI GPIO 4 ---
#define TIME_TO_SLEEP  180         // Timer Deep Sleep 3 Menit
#define uS_TO_S_FACTOR 1000000ULL  
#define ANOMALY_PIN    GPIO_NUM_4  // Pin Interrupt MPU6050

RTC_DATA_ATTR uint32_t counter = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\r\n=================================");
    Serial.println(" HELTEC SENSOR NODE (GPIO 4 INT)");
    Serial.println("=================================");

    // Cek Alasan ESP32 Terbangun
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("[NODE] !!! DETEKSI LONGSOR !!! Bangun Seketika via INT MPU6050!");
    } else {
        Serial.println("[NODE] Bangun Rutin / Booting Awal");
    }

    // Inisialisasi Hardware
    if (!initLoRa()) { Serial.println("[ERROR] LoRa Init Failed!"); }
    if (!initIMU())  { Serial.println("[ERROR] IMU Init Failed!"); }
    if (!initSoil()) { Serial.println("[ERROR] Soil Init Failed!"); }
    setupRain();

    counter++;

    // Pembacaan Data Sensor
    float soil = readSoilPercent();
    IMUData imu = readIMU();
    int jumlahTip = getRainTips();
    float vbat = (analogRead(1) / 4095.0) * 3.3 * 3.2;

    // Print Debug Serial Monitor
    Serial.println("--------------------------------");
    Serial.printf("Counter : %d\r\n", counter);
    Serial.printf("SOIL    : %.1f %%\r\n", soil);
    Serial.printf("Pitch   : %.2f\r\n", imu.pitch);
    Serial.printf("Roll    : %.2f\r\n", imu.roll);
    Serial.printf("Rain    : %d\r\n", jumlahTip);
    Serial.printf("Baterai : %.2f V\r\n", vbat);

    // Rakit & Kirim Payload LoRa
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

    // ============================================================
    // KONFIGURASI DEEP SLEEP & RTC GPIO
    // ============================================================
    
    // 1. Set RTC GPIO 4 ke Pull-Up Internal (Menahan pin di 3.3V saat sleep)
    rtc_gpio_init(ANOMALY_PIN);
    rtc_gpio_set_direction(ANOMALY_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(ANOMALY_PIN);
    rtc_gpio_pullup_en(ANOMALY_PIN);

    // 2. Clear status register MPU6050 sebelum tidur
    readIMU();
    delay(100);

    // 3. Pengecekan Fisik Status Logika Pin GPIO 4
    pinMode(4, INPUT_PULLUP);
    int statusPin4 = digitalRead(4);
    Serial.printf("[CEK HARDWARE] Status Logika GPIO 4: %d (Harus 1/HIGH)\r\n", statusPin4);

    if (statusPin4 == 0) {
        Serial.println("[ERROR] GPIO 4 terbaca LOW (0V)! Cek kabel INT / Resistor Pull-Up ke 3.3V!");
    }

    // 4. Konfigurasi Dual Wakeup (Timer 3 Menit + Trigger LOW (0) GPIO 4)
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(ANOMALY_PIN, 0); // Pemicu 0 (LOW)

    Serial.println("[NODE] Masuk Deep Sleep...");
    Serial.flush();
    
    // 5. Masuk Deep Sleep
    esp_deep_sleep_start();
}

void loop() {
}