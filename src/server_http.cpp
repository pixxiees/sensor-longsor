#include "server_http.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ==========================================
// KONFIGURASI JARINGAN
// ==========================================
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI";

// Ganti IP dengan IP lokal komputermu yang menjalankan Flask (misal: 192.168.1.10)
const char* serverName = "http://192.168.1.10:5000/api/data"; 

void setupWiFi() {
    Serial.println();
    Serial.print("[WiFi] Menghubungkan ke ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
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

void sendDataToServer(int counter, float roll, float pitch, float rain, float soil) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");

        // Format data menjadi JSON secara manual
        String jsonPayload = "{";
        jsonPayload += "\"node_id\":\"NODE_1\",";
        jsonPayload += "\"counter\":" + String(counter) + ",";
        jsonPayload += "\"roll\":" + String(roll) + ",";
        jsonPayload += "\"pitch\":" + String(pitch) + ",";
        jsonPayload += "\"rain\":" + String(rain) + ",";
        jsonPayload += "\"soil_moisture\":" + String(soil);
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