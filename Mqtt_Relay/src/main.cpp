//DEBUGGING OR SERIAL HAS AN BUG IN SOFTAP

#include "namedMesh.h"
#include "DHT.h"
#include "ArduinoJson.h"

//put those values into .env or something like that it's not secure to reveal those values to others

#define   MESH_PREFIX     "Blink"
#define   MESH_PASSWORD   "MokraWoda"
#define   MESH_PORT       5555
#define   HOSTNAME         "MQTT_Temp_Relay"

//DHT Defines
#define DHTPIN 4
#define DHTTYPE DHT11

const int Relay1 = 23;
const int Relay2 = 22;
String nodeName = "Temp_Relay";

// #define PUBPLISHSUFFIX             "painlessMesh/from/#"
// #define SUBSCRIBESUFFIX            "painlessMesh/to/"

// #define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

Scheduler userScheduler; // to control your personal task
namedMesh  mesh;
DHT dht(DHTPIN, DHTTYPE);

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void fetchDht() ;

float h = 0.0;
float t = 0.0;
float hic = 0.0;

const int capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> doc;

String output;


// Task taskFetchDht( TASK_SECOND * 10, TASK_FOREVER, &fetchDht );
// Task taskSendMessage( TASK_SECOND * 10 , TASK_FOREVER, &sendMessage );

void serialize_JSON(){

  doc.clear();
  output.clear();
  // Object["key"] = value
  JsonObject Temp = doc.createNestedObject();
    Temp["temperature"] = t;

  JsonObject Humid = doc.createNestedObject();
    Humid["humidity"] = h;

  JsonObject HIC = doc.createNestedObject();
    HIC["heatIndex"] = hic;

  serializeJson( doc, output );
}

void fetchDht(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  hic = dht.computeHeatIndex(t, h, false);
  serialize_JSON();
}

void sendMessage() {
  mesh.sendSingle( 476456449 , output );
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  if ( from == 476456449 ){
    if (msg == "getDHT") {
      fetchDht();
      mesh.sendBroadcast(output);

    }
    if ( msg == "ON1" ){
      digitalWrite(Relay1, LOW);
      mesh.sendSingle( 76456449 , "Relay 1 state: ON" );
    }
    if ( msg == "OFF1" ){
      digitalWrite(Relay1, HIGH);
      mesh.sendSingle( 76456449 , "Relay 1 state: OFF" );
    }
    if ( msg == "ON2" ){
      digitalWrite(Relay2, LOW);
      mesh.sendSingle( 76456449 , "Relay 2 state: ON" );
    }
    if ( msg == "OFF2" ){
      digitalWrite(Relay2, HIGH);
      mesh.sendSingle( 76456449 , "Relay 2 state: OFF" );
    }
  }
    Serial.println(msg);
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    // Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  // Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  // mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.setName( nodeName );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.setHostname(HOSTNAME);

  dht.begin();

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);

  // userScheduler.addTask( taskFetchDht );
  // taskFetchDht.enable();

  // userScheduler.addTask( taskSendMessage );
  // taskSendMessage.enable();


}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}