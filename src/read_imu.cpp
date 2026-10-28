#include "read_imu.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 41
#define SCL_PIN 42

Adafruit_MPU6050 mpu;

RTC_DATA_ATTR float pitchOffset = 0.0;
RTC_DATA_ATTR float rollOffset = 0.0;
RTC_DATA_ATTR bool isCalibrated = false;

// Bersihkan Latch Register MPU6050 via I2C
void clearMPUInterrupt() {
    Wire.beginTransmission(0x68);
    Wire.write(0x3A); // Register INT_STATUS
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 1);
    if (Wire.available()) { Wire.read(); }
}

bool initIMU()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if(!mpu.begin()) {
        Serial.println("[IMU] MPU6050 NOT FOUND");
        return false;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Setting Interupsi Motion
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(15); 
    mpu.setMotionDetectionDuration(20);  
    mpu.setInterruptPinLatch(false);    
    mpu.setInterruptPinPolarity(true);  // Sinyal HIGH saat terpicu
    mpu.setMotionInterrupt(true);

    if (!isCalibrated) {
        Serial.println("[IMU] Calibrating Offsets...");
        const int SAMPLE = 100;
        float pitchSum = 0, rollSum = 0;

        for(int i = 0; i < SAMPLE; i++) {
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

    float pitch = atan2(data.ay, sqrt(data.ax * data.ax + data.az * data.az)) * 180.0 / PI;
    float roll  = atan2(-data.ax, data.az) * 180.0 / PI;

    data.pitch = pitch - pitchOffset;
    data.roll  = roll - rollOffset;

    clearMPUInterrupt();
    return data;
}