# Is the Air Conditioner On?
## Description:
This project aims to design and deploy an IoT temperature sensing application that can show its measurements in real time in a Node-RED dashboard. Such architecture includes an ESP32 that will do the temperature sensing with a thermistor circuit component, an AWS cloud host server that will communicate with the microncotroller, and a database to store the temperatures collected. A Siri shortcut will be used as an interface as well as a Node-RED dashboard to let the user know if the air conditioner of a specific room is on.
## Project Components & Development Tools:
- Visual Studio Code: source-code editor
- Node-RED: visual interface for IoT device
- MongoDB: database to suport Node-RED dashboard
- Studio 3T: GUI & IDE tool for MongoDB
- MQTT: messaging protocol between Node-RED and IoT device
- ESP32: IoT device to sense temperature signal
- [PubSubClient](https://github.com/knolleary/pubsubclient): allows IoT device to publish/subscribe messaging with a server that supports MQTT
- iOS shortcut: have Siri read the temperature of a specific room by making a call to Node-RED
## Node-RED UI Dashboard:
![gitmicro](https://user-images.githubusercontent.com/70550781/207220129-8a1f6a06-4ee2-4502-b073-cdf93543d0c4.gif)

## Team - Bit Byters:
Luis Meléndez - luis.melendez28@upr.edu

Jesús Hernández - jesus.hernandez19@upr.edu

Hedin García - hedin.garcia@upr.edu
