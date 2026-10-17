#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <Arduino.h>

// Fungsi untuk inisialisasi koneksi Wi-Fi
void setupWiFi();

// Fungsi untuk mengirim data ke server Flask
void sendDataToServer(int counter, float roll, float pitch, float rain, float soil);

#endif