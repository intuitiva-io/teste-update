#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

const char* ssid = "casa1";
const char* password = "Master99@";
const char* updateURL = "https://github.com/cristiano-dev/teste/releases/download/1.0.1/firmware.bin";

void setup() {
    Serial.begin(115200);
    log_i("\n\n\n");
    pinMode(LED_BUILTIN, OUTPUT);
    for (uint8_t t = 4; t > 0; t--) {
        log_i("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    log_i("Conectado na rede %s", WiFi.SSID().c_str());
}

void update_started() {
    Serial.println("CALLBACK: HTTP update process started");
}

void update_finished() {
    Serial.println("CALLBACK: HTTP update process finished");
}

void update_progress(int cur, int total) {
    Serial.printf("CALLBACK: HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
    Serial.printf("CALLBACK: HTTP update fatal error code %d\n", err);
}

void checkForUpdates() {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    
    HTTPUpdate httpUpdate;
    
    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);
	int code;
	String newURL;
    t_httpUpdate_return ret = httpUpdate.update(client, updateURL, "", [&code, &newURL](HTTPClient* client){
		code = client->GET();
		newURL = client->getLocation();
	});
    // Se encontrarmos um redirecionamento, seguiremos a nova URL.
    if (code == 301 || code == 302) {
        log_w("Redirecionando para: %s", newURL.c_str());
        ret = httpUpdate.update(client, newURL);
    }

    switch (ret) {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        checkForUpdates();
        log_i("Aguardando por 10 minutos para a próxima verificação...");
        delay(600000);  // Aguarde 10 minutos (600.000 ms) antes de verificar novamente.
    } else {
        log_i("Não está conectado. Tentando reconectar...");
        WiFi.begin(ssid, password);
        delay(10000);  // Aguarde 10 segundos antes de tentar novamente.
    }
}
