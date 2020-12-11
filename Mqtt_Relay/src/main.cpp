//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "Blink"
#define   MESH_PASSWORD   "MokraWoda"
#define   MESH_PORT       5555

const int Relay1 = 23;
const int Relay2 = 22;

// #define PUBPLISHSUFFIX             "painlessMesh/from/"
// #define SUBSCRIBESUFFIX            "painlessMesh/to/"

// #define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain


Task taskSendMessage( TASK_SECOND * 10 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  String msg = "Hello from relay node ";
  msg += mesh.getNodeId();
  mesh.sendSingle( 476456449 , msg );
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  if ( from == 476456449 ){
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
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  // mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);

  
  userScheduler.addTask( taskSendMessage );
  // taskSendMessage.enable();

}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}