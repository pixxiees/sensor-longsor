#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

void setupWiFi();
void startNewSession();

// Ditambahkan parameter fuzzy_status di bagian akhir
void sendDataToServer(String node_id, int counter, float roll, float pitch, int rain, float soil, float temp, float vbat, float rssi, float snr, String fuzzy_status);

#endif // SERVER_HTTP_H