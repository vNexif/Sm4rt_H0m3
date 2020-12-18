#ifndef _ENVIROMENT_VARS_H_
#define _ENVIROMENT_VARS_H_

//put those values into .env or something like that it's not secure to reveal those values to others
#define MESH_PREFIX "OpCat"
#define MESH_PASSWORD "SlackPussAF"
#define MESH_PORT 5555
#define HOSTNAME "MQTT_Temp_Relay"

String nodeName = "Temp_Relay";
String gatewayName = "Gateway";

//DHT Defines
#define DHTPIN 4
#define DHTTYPE DHT11

const int Relay1 = 23;
const int Relay2 = 22;


#endif