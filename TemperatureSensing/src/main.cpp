#include <WiFi.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <PubSubClient.h>
#include <driver/adc.h>

const char* wifi_network_name = "RUMNET";
const char* wifi_network_password = "Colegio2019";
const char* mqtt_broker_host = "44.212.35.193";
const int mqtt_broker_port = 1883;
const char* publish_topic = "TempData";
const char *subscribe_topic = "Calibration";

WiFiClient esp32Client;
PubSubClient client(esp32Client);

void initialize_wifi() {
  delay(1000);
  Serial.printf("(~)Attempting connection to the \"%s\" Wi-Fi network:\n", wifi_network_name);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_network_name, wifi_network_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.printf("\n(~)Client connected to the \"%s\" Wi-Fi network.", wifi_network_name);
}

void initialize_mqtt() {
  client.setServer(mqtt_broker_host, mqtt_broker_port);
}

void reconnect_mqtt() {
  while(!client.connected()) {
    Serial.printf("\n(~)Attempting connection to MQTT Broker in %s", mqtt_broker_host);
    if (client.connect("ESP32_Client")) {
      Serial.printf("\n(~)Client connected to MQTT Broker in %s!",mqtt_broker_host);
      client.subscribe(subscribe_topic);
    } else {
      Serial.println("\nClient failed to connect to MQTT broker\nWill try again in five seconds");
      delay(5000);
    }
  }
}

void commission() {}

int get_aveg_temp() {
  return 0;
}

void calculate_aveg_temp() {}

void callibration() {}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nMessage received from topic \"%s\": ", topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void publish(const char *pbParameter){
  if (client.connected()) {
    Serial.printf("\nPublishing: %s", pbParameter);
    client.publish(publish_topic, pbParameter);
    // Publish every three seconds
    delay(3000);
  }
}

void setup() {
  Serial.begin(115200);
  // Start Wi-Fi connection
  initialize_wifi();
  // Start MQTT broker connection
  initialize_mqtt();
  client.setCallback(callback);
  delay(2000);
}

void loop() {
  // Make sure there is an MQTT connection
  if(!client.connected()) {
    reconnect_mqtt();
  }
  // Maintain connection with the server and process incoming messages
  client.loop();
  publish("100");
}
