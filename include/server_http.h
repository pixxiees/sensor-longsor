#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

// Fungsi untuk inisialisasi koneksi Wi-Fi
void setupWiFi();

// Fungsi untuk mengirim data ke server Flask
// Ubah deklarasi fungsinya menjadi seperti ini
void sendDataToServer(int counter, float roll, float pitch, int rain, float soil, float temp, float vbat, float rssi, float snr);

#endif