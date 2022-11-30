#include <WiFi.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <PubSubClient.h>
#include <driver/adc.h>

#define NOTSET 0

enum publish_mode {
  PUBLISH_CALIBRATION = 0,
  PUBLISH_TEMPERATURE = 1,
  PUBLISH_DEFAULT = 2,
};

struct calibration_struct {
  int lowest_volt;
  int lowest_temp;
  int highest_volt;
  int highest_temp;
  bool lowestValuesSaved;
  bool highestValuesSaved;
};

const char* wifi_network_name = "RUMNET";
const char* wifi_network_password = "Colegio2019";
const char* mqtt_broker_host = "44.212.35.193";
const int mqtt_broker_port = 1883;
const char* publish_topic = "TempData";
char commission_topic [60];
const char *calibration_topic = "Calibration";
const char *room_assigned;
bool room_was_assigned = false;

WiFiClient esp32Client;
PubSubClient client(esp32Client);

int last_voltage_average;
int voltage_to_publish;

struct calibration_struct calibration_vals;

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
      client.subscribe(commission_topic);
      client.subscribe(calibration_topic);
    } else {
      Serial.println("\nClient failed to connect to MQTT broker\nWill try again in five seconds");
      delay(5000);
    }
  }
}

void commission() {}

int get_avg_temp_voltage() {
  return last_voltage_average;
}

void collect_avg_temp_voltage(void *param) {
  int current_voltage = 0;
  int count_average = 0;
  int voltage_average = 0;
  vTaskDelay(6000 / portTICK_PERIOD_MS); // Allow a wifi & mqtt connection time 
  Serial.print("\nStarting measurements of temperature voltage: ");
  while(1) {
    if (count_average < 10) {
      current_voltage = adc1_get_raw(ADC1_CHANNEL_6);
      Serial.print("\nCurrent voltage value: ");
      Serial.println(current_voltage);
      voltage_average = voltage_average + current_voltage;
      count_average = count_average + 1;
    }
    if (count_average == 10) {
      voltage_average = voltage_average / 10;
      Serial.print("\nAverage value: ");
      Serial.println(voltage_average);
      count_average = 0;
      last_voltage_average = voltage_average;
      voltage_average = 0;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void calibration(int temperature, bool resetValues) {}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nMessage received from topic \"%s\": ", topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if(strcmp(topic, commission_topic) == 0){}
}

void publish(publish_mode mode, const char *pbParameter) {
  char message [90];
  if (client.connected()) {
    switch (mode) {
    case PUBLISH_CALIBRATION:
      sprintf(message, "%s%s",pbParameter," mV");
      break;
    
    case PUBLISH_TEMPERATURE:
      sprintf(message, "%s%s%s","{\"Temperature\": \"",pbParameter,"\"}");
      break;
    default:
      sprintf(message, "%s%s%s","{\"Value\": \"",pbParameter,"\"}");
      break;
    }
    Serial.printf("\nPublishing: %s", message);
    client.publish(publish_topic, message);
    // Publish every three seconds
    delay(3000);
  }
}

void publish(publish_mode mode, int pbParameter) {
  char message[sizeof(int)*8];
  sprintf(message, "%d", pbParameter);
  publish(mode, message);
}

void setup() {
  Serial.begin(115200);
  // Start Wi-Fi connection
  initialize_wifi();
  // Start MQTT broker connection
  initialize_mqtt();
  client.setCallback(callback);
  // Generate a commission topic
  const char* esp32_macaddress = WiFi.macAddress().c_str();
  sprintf(commission_topic, "Commission/%s", esp32_macaddress);
  // Configuration to get values from ESP32
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);
  last_voltage_average = NOTSET;
  // Create task to collect voltage from thermistor
  xTaskCreate(&collect_avg_temp_voltage, "Collect voltage", 2048, NULL, 5, NULL);
  calibration_vals.lowestValuesSaved = false;
  calibration_vals.highestValuesSaved = false;
  delay(2000);
}

void loop() {
  // Make sure there is an MQTT connection
  if(!client.connected()) {
    reconnect_mqtt();
  }
  // Maintain connection with the server and process incoming messages
  client.loop();
  // Publish measurements
  voltage_to_publish = get_avg_temp_voltage();
  if(voltage_to_publish != NOTSET){
    publish(PUBLISH_CALIBRATION, voltage_to_publish);
  }
}
