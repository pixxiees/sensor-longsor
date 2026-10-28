#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

// Deklarasi fungsi-fungsi HTTP
void setupWiFi();
void startNewSession(); // <--- PASTI KAN BARIS INI ADA
void sendDataToServer(String node_id, int counter, float roll, float pitch, 
                      int rain, float soil, float temp, float vbat, 
                      float rssi, float snr);

#endif