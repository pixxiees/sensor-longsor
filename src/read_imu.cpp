#include "read_imu.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 41
#define SCL_PIN 42
#define MPU6050_ADDR 0x68

Adafruit_MPU6050 mpu;

RTC_DATA_ATTR float pitchOffset = 0.0;
RTC_DATA_ATTR float rollOffset  = 0.0;
RTC_DATA_ATTR float gyroYOffset = 0.0;
RTC_DATA_ATTR float currentYaw  = 0.0;
RTC_DATA_ATTR bool isCalibrated = false;

void writeMPURegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void clearMPUInterrupt() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3A); 
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 1);
    while (Wire.available()) { 
        Wire.read(); 
    }
}

bool initIMU()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!mpu.begin()) {
        Serial.println("[IMU] MPU6050 NOT FOUND");
        return false;
    }

    // 1. MATIKAN High-Pass Filter dulu agar pembacaan gravitasi (DC) murni & stabil!
    mpu.setHighPassFilter(MPU6050_HIGHPASS_DISABLE);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    delay(100);

    // --- KALIBRASI OFFSET SAAT BOOTING AWAL ---
    if (!isCalibrated) {
        Serial.println("[IMU] Tunggu 3 detik agar modul diam sebelum kalibrasi...");
        delay(3000);

        Serial.println("[IMU] Calibrating Offsets (Roll=Z, Pitch=X, Gravity=Y)...");
        const int SAMPLE = 100;
        float pitchSum = 0, rollSum = 0, gyroYSum = 0;

        for (int i = 0; i < SAMPLE; i++) {
            sensors_event_t accel, gyro, temp;
            mpu.getEvent(&accel, &gyro, &temp);

            float ax = accel.acceleration.x;
            float ay = accel.acceleration.y;
            float az = accel.acceleration.z;

            float roll  = atan2(az, ay) * 180.0 / PI;
            float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

            rollSum  += roll;
            pitchSum += pitch;
            gyroYSum += gyro.gyro.y;
            delay(10);
        }

        rollOffset   = rollSum / SAMPLE;
        pitchOffset  = pitchSum / SAMPLE;
        gyroYOffset  = gyroYSum / SAMPLE;
        currentYaw   = 0.0;
        isCalibrated = true;

        Serial.printf("[IMU] Offset Tersimpan -> Roll: %.2f, Pitch: %.2f\r\n", rollOffset, pitchOffset);
    }

    // 2. Konfigurasi Interrupt SETELAH kalibrasi selesai
    writeMPURegister(0x37, 0x80); // Active LOW, Push-Pull, 50us Pulse
    writeMPURegister(0x1F, 30);   // Motion Threshold ~60mg
    writeMPURegister(0x20, 10);   // Duration 10ms
    writeMPURegister(0x38, 0x40); // Enable Motion Interrupt

    clearMPUInterrupt();
    Serial.println("[IMU] READY");
    return true;
}

IMUData readIMU()
{
    sensors_event_t accel, gyro, temp;
    IMUData data;

    // Pastikan DHPF mati saat membaca kemiringan statis agar tidak ada offset transien
    mpu.setHighPassFilter(MPU6050_HIGHPASS_DISABLE);
    delay(50);

    const int READ_SAMPLES = 30;
    float sumAx = 0, sumAy = 0, sumAz = 0;
    float sumGx = 0, sumGy = 0, sumGz = 0;

    unsigned long prevTime = millis();

    for (int i = 0; i < READ_SAMPLES; i++) {
        mpu.getEvent(&accel, &gyro, &temp);
        
        unsigned long now = millis();
        float dt = (now - prevTime) / 1000.0;
        prevTime = now;

        sumAx += accel.acceleration.x;
        sumAy += accel.acceleration.y;
        sumAz += accel.acceleration.z;
        sumGx += gyro.gyro.x;
        sumGy += gyro.gyro.y;
        sumGz += gyro.gyro.z;

        float rateYaw = (gyro.gyro.y - gyroYOffset) * 180.0 / PI;
        if (abs(rateYaw) > 0.5) {
            currentYaw += rateYaw * dt;
        }

        delay(2);
    }

    data.ax = sumAx / READ_SAMPLES;
    data.ay = sumAy / READ_SAMPLES;
    data.az = sumAz / READ_SAMPLES;
    data.gx = sumGx / READ_SAMPLES;
    data.gy = sumGy / READ_SAMPLES;
    data.gz = sumGz / READ_SAMPLES;
    data.temperature = temp.temperature;

    float roll  = atan2(data.az, data.ay) * 180.0 / PI;
    float pitch = atan2(-data.ax, sqrt(data.ay * data.ay + data.az * data.az)) * 180.0 / PI;

    data.roll  = roll - rollOffset;
    data.pitch = pitch - pitchOffset;
    data.yaw   = currentYaw;

    // Aktifkan kembali High-Pass Filter 0.63Hz HANYA untuk deteksi gerakan saat deep sleep
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    delay(20);
    clearMPUInterrupt();

    return data;
}