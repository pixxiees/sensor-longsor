#ifndef READ_IMU_H
#define READ_IMU_H

struct IMUData
{
    float ax;
    float ay;
    float az;

    float gx;
    float gy;
    float gz;

    float pitch;
    float roll;
    float yaw;

    float temperature;
};

bool initIMU();
IMUData readIMU();

#endif