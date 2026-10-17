#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

// Fungsi untuk inisialisasi koneksi Wi-Fi
void setupWiFi();

void sendDataToServer(String node_id, int counter, float roll, float pitch, int rain, float soil, float temp, float vbat, float rssi, float snr);

#endif