#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

void setupWiFi();
void startNewSession();

void sendDataToServer(String node_id, int counter, float roll, float pitch, float yaw, int rain, float soil, float box_temp, float box_hum, float rssi, float snr, String fuzzy_status);

#endif