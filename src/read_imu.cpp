#include "read_imu.h"

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 41
#define SCL_PIN 42

Adafruit_MPU6050 mpu;

float pitchOffset = 0.0;
float rollOffset = 0.0;

bool initIMU()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if(!mpu.begin())
    {
        Serial.println("[IMU] MPU6050 NOT FOUND");
        // return false;
        return true;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("[IMU] Calibrating...");

    const int SAMPLE = 200;

    float pitchSum = 0;
    float rollSum = 0;

    for(int i = 0; i < SAMPLE; i++)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        mpu.getEvent(&accel, &gyro, &temp);

        float ax = accel.acceleration.x;
        float ay = accel.acceleration.y;
        float az = accel.acceleration.z;

        float pitch = atan2(
            ay,
            sqrt(ax * ax + az * az)
        ) * 180.0 / PI;

        float roll = atan2(
            -ax,
            az
        ) * 180.0 / PI;

        pitchSum += pitch;
        rollSum += roll;

        delay(10);
    }

    pitchOffset = pitchSum / SAMPLE;
    rollOffset = rollSum / SAMPLE;

    Serial.println("[IMU] Calibration Finished");

    Serial.println("[IMU] READY");

    return true;
}

IMUData readIMU()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    IMUData data;

    mpu.getEvent(&accel, &gyro, &temp);

    data.ax = accel.acceleration.x;
    data.ay = accel.acceleration.y;
    data.az = accel.acceleration.z;

    data.gx = gyro.gyro.x;
    data.gy = gyro.gyro.y;
    data.gz = gyro.gyro.z;

    data.temperature = temp.temperature;

    float pitch = atan2(
        data.ay,
        sqrt(data.ax * data.ax + data.az * data.az)
    ) * 180.0 / PI;

    float roll = atan2(
        -data.ax,
        data.az
    ) * 180.0 / PI;

    data.pitch = pitch - pitchOffset;
    data.roll = roll - rollOffset;

    return data;
}