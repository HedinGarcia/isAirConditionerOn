#include <WiFi.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <PubSubClient.h>
#include <driver/adc.h>

#define NOTSET 0

enum publish_mode {
  PUBLISH_ROOM_ASSIGNED = 0,
  PUBLISH_CALIBRATION,
  PUBLISH_TEMPERATURE,
  PUBLISH_DEFAULT
};

struct calibration_struct {
  int lowest_volt;
  int lowest_temp;
  int highest_volt;
  int highest_temp;
  bool lowestValuesSaved;
  bool highestValuesSaved;
};

const char* wifi_network_name = "Robotica squad";
const char* wifi_network_password = "Mayaguez802";
const char* mqtt_broker_host = "44.212.35.193";
const int mqtt_broker_port = 1883;
char commission_sub_topic [60];
char calibration_sub_topic[60];
char calibration_pub_topic[60];
char room_assigned_pub_topic[60];
char voltage_pub_topic[60];
const char *room_assigned;
bool room_was_assigned;
bool device_calibrated;
const char* esp32_macaddress;

WiFiClient esp32Client;
PubSubClient client(esp32Client);

int last_voltage_average;
int voltage_to_publish;

struct calibration_struct calibration_vals;

int get_avg_temp_voltage();
void collect_avg_temp_voltage(void *param);
void calibration(int temperature, bool resetValues);
void publish(publish_mode mode, const char *publish_topic, int pbParameter);
void publish(publish_mode mode, const char *publish_topic, calibration_struct pbParameter);
void publish(publish_mode mode, const char *publish_topic, const char *pbParameter);

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
    if (client.connect(esp32_macaddress)) {
      Serial.printf("\n(~)Client connected to MQTT Broker in %s!",mqtt_broker_host);
      client.subscribe(commission_sub_topic);
      client.subscribe(calibration_sub_topic);
    } else {
      Serial.println("\nClient failed to connect to MQTT broker\nWill try again in five seconds");
      delay(5000);
    }
  }
}

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

void calibration(int temperature, bool resetValues) {
  if(!room_was_assigned) return;
  int voltage = get_avg_temp_voltage();
  if(resetValues){
    calibration_vals.lowest_volt = 0;
    calibration_vals.lowest_temp = 0;
    calibration_vals.highest_volt = 0;
    calibration_vals.highest_temp = 0;
    calibration_vals.lowestValuesSaved = false;
    calibration_vals.highestValuesSaved = false;
    device_calibrated = false;
  }
  else if(calibration_vals.lowestValuesSaved == false && calibration_vals.highestValuesSaved == false) {
    calibration_vals.lowest_volt = voltage;
    calibration_vals.lowest_temp = temperature;
    calibration_vals.lowestValuesSaved = true;
  }
  else if(calibration_vals.lowestValuesSaved == true && calibration_vals.highestValuesSaved == false) {
    if(calibration_vals.lowest_volt > voltage || calibration_vals.lowest_temp > temperature){
      int temp_voltage = calibration_vals.lowest_volt;
      int temp_temperature = calibration_vals.lowest_temp;
      calibration_vals.lowest_volt = voltage;
      calibration_vals.lowest_temp = temperature;
      calibration_vals.highest_volt = temp_voltage;
      calibration_vals.highest_temp = temp_temperature;
    }
    else{
      calibration_vals.highest_volt = voltage;
      calibration_vals.highest_temp = temperature;
    }
    calibration_vals.highestValuesSaved = true;
    device_calibrated = true;
  }
  publish(PUBLISH_CALIBRATION, calibration_pub_topic, calibration_vals);
}

void publish(publish_mode mode, const char *publish_topic, calibration_struct pbParameter) {
  char message[sizeof(calibration_struct)*8];
  sprintf(message, "{\"lowestVolt\": %d, \"lowestTemp\": %d, \"highestVolt\": %d, \"highestTemp\": %d}", calibration_vals.lowest_volt, calibration_vals.lowest_temp, calibration_vals.highest_volt, calibration_vals.highest_temp);
  publish(mode, publish_topic, message);
}

void publish(publish_mode mode, const char *publish_topic, int pbParameter) {
  char message[sizeof(int)*8];
  sprintf(message, "%d", pbParameter);
  publish(mode, publish_topic, message);
}

void publish(publish_mode mode, const char *publish_topic, const char *pbParameter) {
  char message [90];
  if (client.connected()) {
    switch (mode) {
    case PUBLISH_ROOM_ASSIGNED:
      sprintf(message, "%s", pbParameter);
      break;
    case PUBLISH_CALIBRATION:
      sprintf(message, "%s",pbParameter);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("\nMessage received from topic \"%s\": ", topic);
  char buffer[length+1];
  for (int i = 0; i < length; i++) {
    buffer[i] = (char) payload[i];
    Serial.print((char)payload[i]);
  }
  buffer[length] = '\0';
  if(strcmp(topic, commission_sub_topic) == 0){
    if(room_was_assigned == false){
      room_assigned = buffer;
      room_was_assigned = true;
      publish(PUBLISH_ROOM_ASSIGNED, room_assigned_pub_topic, room_assigned);
    }
  }
  else if(strcmp(topic, calibration_sub_topic) == 0){
    if(strcmp(buffer, "reset") == 0){
        calibration(0, true);
    }
    else if(device_calibrated == false){
      calibration(atoi(buffer),false);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Start Wi-Fi connection
  initialize_wifi();
  // Start MQTT broker connection
  initialize_mqtt();
  client.setCallback(callback);
  // Generate subscribe and publish topics
  esp32_macaddress = WiFi.macAddress().c_str();
  sprintf(commission_sub_topic, "Commission/%s", esp32_macaddress);
  sprintf(room_assigned_pub_topic, "AssignedRoom/%s", esp32_macaddress);
  sprintf(calibration_sub_topic, "Calibration/%s", esp32_macaddress);
  sprintf(calibration_pub_topic, "CalibrationPub/%s", esp32_macaddress);
  sprintf(voltage_pub_topic, "VoltData/%s", esp32_macaddress);
  // Configuration to get values from ESP32
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);
  room_was_assigned = false;
  device_calibrated = false;
  last_voltage_average = NOTSET;
  // Create task to collect voltage from thermistor
  xTaskCreate(&collect_avg_temp_voltage, "Collect voltage", 2048, NULL, 5, NULL);
  calibration_vals.lowest_volt = 0;
  calibration_vals.lowest_temp = 0;
  calibration_vals.highest_volt = 0;
  calibration_vals.highest_temp = 0;
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
    publish(PUBLISH_CALIBRATION, voltage_pub_topic, voltage_to_publish);
  }
}
