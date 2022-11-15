#include <WiFi.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <PubSubClient.h>
#include <driver/adc.h>

const char *wifi_network_name = "RUMNET";
const char *wifi_network_password = "Colegio2019";

void connect_mqtt() {}

void initialize_wifi() {
  delay(1000);
  Serial.printf("Creating a connection to the \"%s\" Wi-Fi network:\n", wifi_network_name);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_network_name, wifi_network_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.printf("\nConnection made to the \"%s\" Wi-Fi network.\n", wifi_network_name);
}

void initialize_mqtt() {
}

void commission() {}

int get_aveg_temp() {
  return 0;
}

void calculate_aveg_temp() {}

void callibration() {}

void callback(char *topic, byte *payload, unsigned int length) {}

void measure_and_publish(void *pvParameter) {}

void setup() {
  Serial.begin(115200);
  initialize_wifi();
}

void loop() {}