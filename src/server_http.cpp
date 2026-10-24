#include "server_http.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ==========================================
// KONFIGURASI JARINGAN
// ==========================================
const char* ssid = "bebek madura";
const char* password = "kopihazel";

// Ganti IP dengan IP lokal komputermu yang menjalankan Flask (misal: 192.168.1.10)
const char* serverName = "http://pixxelee.pythonanywhere.com/api/data";

void setupWiFi() {
    Serial.println();
    Serial.print("[WiFi] Menghubungkan ke ");
    Serial.println(ssid);

    // 1. Matikan mode hemat daya WiFi agar pemancar bekerja penuh (100% Power)
    WiFi.setSleep(false);
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) { // Naikkan timeout ke 15 detik
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Terhubung!");
        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] Gagal terhubung (Timeout).");
    }
}

void sendDataToServer(String node_id, int counter, float roll, float pitch, int rain, float soil, float temp, float vbat, float rssi, float snr) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi terputus, mencoba menghubungkan ulang...");
        WiFi.reconnect();
        int wait = 0;
        while (WiFi.status() != WL_CONNECTED && wait < 10) {
            delay(500);
            wait++;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.setTimeout(5000); 
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Connection", "close"); 

        String jsonPayload = "{";
        jsonPayload += "\"node_id\":\"" + node_id + "\","; 
        jsonPayload += "\"counter\":" + String(counter) + ",";
        jsonPayload += "\"roll\":" + String(roll) + ",";
        jsonPayload += "\"pitch\":" + String(pitch) + ",";
        jsonPayload += "\"rain\":" + String(rain) + ",";
        jsonPayload += "\"soil_moisture\":" + String(soil) + ",";
        jsonPayload += "\"temp\":" + String(temp) + ",";
        jsonPayload += "\"vbat\":" + String(vbat) + ",";
        jsonPayload += "\"rssi\":" + String(rssi) + ",";
        jsonPayload += "\"snr\":" + String(snr);
        jsonPayload += "}";

        Serial.print("[HTTP] Mengirim data... ");
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.printf("Sukses (Code: %d)\n", httpResponseCode);
        } else {
            Serial.printf("Error (Code: %d) - %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
        }
        
        http.end();
    } else {
        Serial.println("[HTTP] Batal mengirim, WiFi terputus.");
        // Opsi: Panggil setupWiFi() lagi di sini jika ingin auto-reconnect
    }
}