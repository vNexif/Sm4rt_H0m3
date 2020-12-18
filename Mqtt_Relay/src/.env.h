#ifndef _ENVIROMENT_VARS_H_
#define _ENVIROMENT_VARS_H_

//put those values into .env or something like that it's not secure to reveal those values to others
#define MESH_PREFIX "MESH_PREFIX"
#define MESH_PASSWORD "MESH_PASSWORD"
#define MESH_PORT 5555
#define HOSTNAME "HOSTNAME"

String nodeName = "nodeName";
String gatewayName = "gatewayName";

//DHT Defines
#define DHTPIN 4
#define DHTTYPE DHT11

const int Relay1 = 23;
const int Relay2 = 22;

#endif