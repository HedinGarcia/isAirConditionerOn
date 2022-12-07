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
- [Is Air Conditioner On Website](http://44.212.35.193:1880/ui/#!/0?socketid=ZmLaZWwm6VzPmOrtAABj)
<img width="1512" alt="Screenshot 2022-12-06 at 10 05 37 PM" src="https://user-images.githubusercontent.com/70550781/206070007-3ea6d882-30e1-4138-b899-bb8ebd81c2f2.png">

## Team - Bit Byters:
Luis Meléndez - luis.melendez28@upr.edu

Jesús Hernández - jesus.hernandez19@upr.edu

Hedin García - hedin.garcia@upr.edu
