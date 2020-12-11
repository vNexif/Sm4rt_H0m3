//DEBUGGING OR SERIAL HAS AN BUG IN SOFTAP

#include "namedMesh.h"
#include "PubSubClient.h"
#include "Button2.h"
#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>


// PainlessMesh credentials ( name, password and port ): You should change these
#define   MESH_PREFIX     "Blink"
#define   MESH_PASSWORD   "MokraWoda"
#define   MESH_PORT       5555

// WiFi credentials: should match your access point!
#define   STATION_SSID     "Penetrator"
#define   STATION_PASSWORD "wchujkurwaluj"

#define   HOSTNAME         "MQTT_Bridge"

Scheduler userScheduler;   // to control your personal task

namedMesh  mesh;
WiFiClient wifiClient;
AsyncWebServer server(80);

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

String nodeName = "Gateway";


// hivemq pubblic broker address and port
char mqttBroker[]  = "192.168.5.1";
#define MQTTPORT 1883

// topic's suffix: everyone can publish/subscribe to this public broker,
// you have to change the following 2 defines
#define PUBPLISHSUFFIX             "painlessMesh/from/"
#define SUBSCRIBESUFFIX            "painlessMesh/to/"

#define PUBPLISHFROMGATEWAYSUFFIX  PUBPLISHSUFFIX "gateway"

#define CHECKCONNDELTA 60     // check interval ( seconds ) for mqtt connection

PubSubClient mqttClient;

bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
uint32_t nexttime=0;
uint8_t  initialized=0;




// messages received from the mqtt broker
void mqttCallback(char* topic, uint8_t* payload, unsigned int length) 
  {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  // Serial.printf("mc t:%s  p:%s\n", topic, payload);
  
  String targetStr = String(topic).substring(strlen(SUBSCRIBESUFFIX));
  if(targetStr == "gateway")
    {
    if(msg == "getNodes")
      {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish( PUBPLISHFROMGATEWAYSUFFIX, str.c_str() );
      }
    if(msg == "getrt")
      {
      mqttClient.publish( PUBPLISHFROMGATEWAYSUFFIX, mesh.subConnectionJson(false).c_str() );
      }
    if(msg == "asnodetree")
      {
      //mqttClient.publish( PUBPLISHFROMGATEWAYSUFFIX, mesh.asNodeTree().c_str() );
      }
    }
  else if(targetStr == "broadcast") 
    {
    mesh.sendBroadcast( msg );
    }
  else
    {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
      {
      mesh.sendSingle(target, msg);
      }
    else
      {
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX, "Client not connected!");
      }
    }
  }





// Needed for painless library

// messages received from painless mesh network
void receivedCallback( const uint32_t &from, const String &msg ) 
  {
  // Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = PUBPLISHSUFFIX + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
  }




void newConnectionCallback(uint32_t nodeId) 
  {
  // Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  // Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }




void changedConnectionCallback() 
  {
  // Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) 
    {
    // Serial.printf(" %u", *node);
    node++;
    }
  Serial.println();
  calc_delay = true;
  }




void nodeTimeAdjustedCallback(int32_t offset) 
  {
  // Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(),offset);
  }




void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
  {
  // Serial.printf("Delay from node:%u delay = %d\n", nodeId,delay);
  }




void reconnect()
{
  //byte mac[6];
  char MAC[9];
  int i;

  // unique string
  //WiFi.macAddress(mac);
  //sprintf(MAC,"%02X",mac[2],mac[3],mac[4],mac[5]);
  sprintf(MAC, "%08X",(uint32_t)ESP.getEfuseMac());  // generate unique addresss.
  // Loop until we're reconnected
  while (!mqttClient.connected()) 
    {
    // Serial.println("Attempting MQTT connection...");    
    // Attemp to connect
    if (mqttClient.connect(/*MQTT_CLIENT_NAME*/MAC)) 
      {
      // Serial.println("Connected");  
      mqttClient.publish(PUBPLISHFROMGATEWAYSUFFIX,"Ready!");
      mqttClient.subscribe(SUBSCRIBESUFFIX "#");
      } 
    else
      {
      // Serial.print("Failed, rc=");
      // Serial.print(mqttClient.state());
      // Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
      mesh.update();
      mqttClient.loop();
      }
    }
}






IPAddress getlocalIP() 
  {
  return IPAddress(mesh.getStationIP());
  }





String scanprocessor(const String& var)
{
  if(var == "SCAN")
    return mesh.subConnectionJson(false) ;
  return String();
}





void setup() 
  {
  // Serial.begin(115200);


  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  // mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 1. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA );
  mesh.setName( nodeName );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    if (request->hasArg("BROADCAST"))
      {
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
      }
    });
  server.on("/map", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send_P(200, "text/html", "<html><head><script type='text/javascript' src='https://cdnjs.cloudflare.com/ajax/libs/vis/4.21.0/vis.js'></script><link href='https://cdnjs.cloudflare.com/ajax/libs/vis/4.21.0/vis-network.min.css' rel='stylesheet' type='text/css' /><style type='text/css'>#mynetwork {width: 1024px;height: 768px;border: 1px solid lightgray;}</style></head><body><h1>PainlessMesh Network Map</h1><div id='mynetwork'></div><a href=https://visjs.org>Made with vis.js<img src='http://www.davidefabbri.net/files/visjs_logo.png' width=40 height=40></a><script>var txt = '%SCAN%';</script><script type='text/javascript' src='http://www.davidefabbri.net/files/painlessmeshmap.js'></script></body></html>", scanprocessor );
    });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "application/json", mesh.subConnectionJson(false) );
    });
  /*server.on("/asnodetree", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "text/html", mesh.asNodeTree().c_str() );
    });*/
  server.begin();

  // if you want your node to accept OTA firmware, simply include this line
  // with whatever role you want your hardware to be. For instance, a
  // mesh network may have a thermometer, rain detector, and bridge. Each of
  // those may require different firmware, so different roles are preferrable.
  //
  // MAKE SURE YOUR UPLOADED OTA FIRMWARE INCLUDES OTA SUPPORT OR YOU WILL LOSE
  // THE ABILITY TO UPLOAD MORE FIRMWARE OVER OTA. YOU ALSO WANT TO MAKE SURE
  // THE ROLES ARE CORRECT
  mesh.initOTAReceive("bridge");
  
  mqttClient.setServer(mqttBroker, MQTTPORT);
  mqttClient.setCallback(mqttCallback);  
  mqttClient.setClient(wifiClient);
  }






void loop() 
  {
  // it will run the user scheduler as well
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP())
    {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    initialized = 1;
    }
  if ( ( millis() >= nexttime ) && ( initialized ) )
    {
    nexttime=millis()+CHECKCONNDELTA*1000;
    if (!mqttClient.connected()) 
      {reconnect();}
    }
  }