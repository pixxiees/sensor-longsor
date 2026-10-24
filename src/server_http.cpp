#include "server_http.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "bebek madura";
const char* password = "kopihazel";
const char* serverName = "http://pixxelee.pythonanywhere.com/api/data";

void setupWiFi() {
    Serial.println();
    Serial.print("[WiFi] Menghubungkan ke ");
    Serial.println(ssid);

    // Wajib matikan sleep mode agar pemancar WiFi selalu di performa penuh (100% Power)
    WiFi.setSleep(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Kunci proses setup sampai WiFi benar-benar terhubung 100%
    int dots = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        dots++;
        if (dots % 20 == 0) Serial.println();
    }

    Serial.println("\n[WiFi] Terhubung Penuh!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
}

void sendDataToServer(String node_id, int counter, float roll, float pitch, int rain, float soil, float temp, float vbat, float rssi, float snr) {
    // Reconnect cepat jika koneksi WiFi terputus
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] WiFi terputus, mencoba reconnect cepat...");
        WiFi.reconnect();
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 10) {
            delay(200);
            timeout++;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        
        // Timeout diturunkan ke 2000ms agar penerimaan LoRa tidak terganggu lama
        http.setTimeout(2000); 
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Connection", "close"); // Menutup TCP Socket setelah kirim

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

        Serial.printf("[HTTP] Mengirim Counter %d... ", counter);
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.printf("BERHASIL (Code: %d)\n", httpResponseCode);
        } else {
            Serial.printf("GAGAL (Error: %d) - %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
        }
        
        http.end();
    } else {
        Serial.printf("[HTTP] Skip Counter %d: WiFi gagal terhubung.\n", counter);
    }
}