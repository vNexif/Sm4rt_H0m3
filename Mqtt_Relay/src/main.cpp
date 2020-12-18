//DEBUGGING OR SERIAL HAS AN BUG IN SOFTAP

#include "namedMesh.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include ".env.h"

Scheduler userScheduler; // to control your personal task
namedMesh mesh;
DHT dht(DHTPIN, DHTTYPE);

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain
void fetchDht();
void send_DHT();
void get_Functions();
void get_Help();

// Task t1(TASK_IMMEDIATE, TASK_SCHEDULE, &fetchDht, &userScheduler);
// Task taskSendMessage( TASK_SECOND * 10 , TASK_FOREVER, &sendMessage );

//Functions
std::map<std::string, bool> node_function =
    {
        {"temperature", true},
        {"humidity", true},
        {"hic", true},
        {"bar", false},
        {"mov", false}};

void get_Help()
{ 
  String s_help = String("To get my functionality send me 'getFunc' \nTo get my dht sensor data send me 'getDHT' \nTo turn ON/OFF the relay send me 'ON|OFF'(1|2)");
  mesh.sendSingle(gatewayName, s_help);
}

void get_Functions()
{
  DynamicJsonDocument j_funcs(4096);
  String js_funcs;
  j_funcs.clear(), js_funcs.clear();

  JsonObject meta = j_funcs.createNestedObject("nodeMeta");
  meta["nodeID"] = mesh.getNodeId();
  meta["nodeName"] = mesh.getName();
  JsonObject meta_Place = meta.createNestedObject("nodePlace");
  meta_Place["area"] = "NexRoom";
  meta_Place["place"] = "Magiczna_Szafa";

  JsonObject node_func = j_funcs.createNestedObject("nodeFunctions");
  node_func["temperature"] = node_function["temperature"];
  node_func["humidity"] = node_function["humidity"];
  node_func["hic"] = node_function["hic"];
  node_func["barometer"] = node_function["bar"];
  node_func["movement"] = node_function["mov"];

  serializeJson(j_funcs, js_funcs);
  mesh.sendSingle(gatewayName, js_funcs);
}

void send_DHT(float temperature, float humidity, float hic)
{
  DynamicJsonDocument j_dht(4096);
  String js_dht;
  Serial.println("Clearing JSON buffer");
  j_dht.clear(), js_dht.clear();

  JsonObject data = j_dht.createNestedObject("data");
  data["temperature"] = temperature;
  data["humidity"] = humidity;
  data["heatIndex"] = hic;

  serializeJson(j_dht, js_dht);
  mesh.sendSingle( gatewayName, js_dht);
  // Serial.println("JSON GUT");
}

void fetchDht()
{
  float humidity = {0.0}, temperature = {0.0}, hic = {0.0};
  Serial.println("Fetching dHT...");
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  hic = dht.computeHeatIndex(temperature, humidity, false);
  send_DHT(temperature, humidity, hic);
}

void sendMessage()
{
}

// Needed for painless library
void receivedCallback(String &from, String &msg)
{
  if (from == "Gateway")
  {
    if (msg == "?")
    {
      get_Help();
    }
    if (msg == "getFunc")
    {
      get_Functions();
    }
    if (msg == "getDHT")
    {
      fetchDht();
    }
    if (msg == "ON1")
    {
      digitalWrite(Relay1, LOW);
      String reply = String ("Relay 1 state: ON");
      mesh.sendSingle(gatewayName, reply);
    }
    if (msg == "OFF1")
    {
      digitalWrite(Relay1, HIGH);
      String reply = String ("Relay 1 state: OFF");
      mesh.sendSingle(gatewayName, reply);
    }
    if (msg == "ON2")
    {
      digitalWrite(Relay2, LOW);
      String reply = String ("Relay 2 state: ON");
      mesh.sendSingle(gatewayName, reply);
    }
    if (msg == "OFF2")
    {
      digitalWrite(Relay2, HIGH);
      String reply = String ("Relay 2 state: OFF");
      mesh.sendSingle(gatewayName, reply);
    }
  }
  Serial.println(msg);
  // Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId)
{
  // Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  // Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  // Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup()
{
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  // mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.setName(nodeName);
  mesh.setHostname(HOSTNAME);
  mesh.setContainsRoot(true);

  dht.begin();

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // userScheduler.addTask( taskFetchDht );
  // taskFetchDht.enable();

  // userScheduler.addTask( taskSendMessage );
  // taskSendMessage.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}