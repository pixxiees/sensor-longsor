#include "read_imu.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 41
#define SCL_PIN 42
#define MPU6050_ADDR 0x68

// Deklarasi Objek MPU6050
Adafruit_MPU6050 mpu;

RTC_DATA_ATTR float pitchOffset = 0.0;
RTC_DATA_ATTR float rollOffset = 0.0;
RTC_DATA_ATTR bool isCalibrated = false;

// Tulis Register Manual via I2C
void writeMPURegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

// Clear Interrupt Latch via Baca Register 0x3A (INT_STATUS)
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

    // --- KONFIGURASI REGISTER INTERRUPT MPU6050 (DIRECT REGISTER) ---
    
    // 1. High Pass Filter ke 0.63Hz
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);

    // 2. Register 0x37 (INT_PIN_CFG): Active LOW, Push-Pull, Latch Interrupt
    writeMPURegister(0x37, 0xA0); 

    // 3. Register 0x38 (INT_ENABLE): Aktifkan Motion Detection Interrupt
    writeMPURegister(0x38, 0x40); 

    // 4. THRESHOLD & DURATION DIUBAH AGAR SANGAT SENSITIF:
    writeMPURegister(0x1F, 15);  // Nilai rekomendasi awal (32 mg/LSB)
    writeMPURegister(0x20, 2);  // Durasi 1ms (Respon instan tanpa penundaan)

    // --- KALIBRASI OFFSET (BOOTING AWAL) ---
    if (!isCalibrated) {
        Serial.println("[IMU] Calibrating Offsets...");
        const int SAMPLE = 100;
        float pitchSum = 0, rollSum = 0;

        for (int i = 0; i < SAMPLE; i++) {
            sensors_event_t accel, gyro, temp;
            mpu.getEvent(&accel, &gyro, &temp);

            float ax = accel.acceleration.x;
            float ay = accel.acceleration.y;
            float az = accel.acceleration.z;

            float pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
            float roll  = atan2(-ax, az) * 180.0 / PI;

            pitchSum += pitch;
            rollSum += roll;
            delay(10);
        }

        pitchOffset = pitchSum / SAMPLE;
        rollOffset  = rollSum / SAMPLE;
        isCalibrated = true;
    }

    clearMPUInterrupt();
    Serial.println("[IMU] READY");
    return true;
}

IMUData readIMU()
{
    sensors_event_t accel, gyro, temp;
    IMUData data;

    mpu.getEvent(&accel, &gyro, &temp);

    data.ax = accel.acceleration.x;
    data.ay = accel.acceleration.y;
    data.az = accel.acceleration.z;
    data.gx = gyro.gyro.x;
    data.gy = gyro.gyro.y;
    data.gz = gyro.gyro.z;
    data.temperature = temp.temperature;

    // Saja diperbaiki: data.ax * data.ax
    float pitch = atan2(data.ay, sqrt(data.ax * data.ax + data.az * data.az)) * 180.0 / PI;
    float roll  = atan2(-data.ax, data.az) * 180.0 / PI;

    data.pitch = pitch - pitchOffset;
    data.roll  = roll - rollOffset;

    clearMPUInterrupt();
    return data;
}