
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define STATIONS 4
#define ON 1
#define OFF 0

byte stationRelays[STATIONS] =  {
  2, /* RELAY 1 */
  7, /* RELAY 2 */
  8, /* RELAY 3 */
  9  /* RELAY 4 */
};

const unsigned long status_frequency = 60000; // every 60 seconds
unsigned long lastStatusSendTime;
unsigned long currentTime;

static byte mac[] = { 0x5E, 0xED, 0x52, 0xFE, 0xED, 0x00 };
IPAddress host_ip(10, 0, 1, 201);
byte mqtt_broker[] = { 10, 0, 1, 181 };

EthernetClient ethClient;
PubSubClient client(mqtt_broker, 1883, messageCallback, ethClient);

void messageCallback(char* topic, byte* payload, unsigned int length) {
  int station = getStation(topic);
  int state = getState(payload, length);
  setStationRelayState(station, state);
}

void setup() {
  Serial.begin(9600);
  for(int i = 0; i < STATIONS; i++) {
    pinMode(stationRelays[i], OUTPUT);
  }
  Ethernet.begin(mac, host_ip);
  if (client.connect("irrigation")) {
    client.subscribe("command/#"); // Subscribe to all station commands
    Serial.println("connected and subscribed");
  }
  lastStatusSendTime = 0;
}

void loop() {
  client.loop();
  if (statusDue()) {
    sendStatus();
  } 
}

void sendStatus() {
  for(int stationID = 1; stationID <= STATIONS; stationID++) {
    client.publish(topic(stationID), stationStateMessage(stationID));
  }
  lastStatusSendTime = millis();
}

int getStation(char* topic) {
  String topicString = String(topic);
  int slashIndex = topicString.lastIndexOf("/");
  return topicString.substring(slashIndex + 1).toInt();
}

int getState(byte* payload, int length) {
  if (strncmp((const char*)payload, "on", length) == 0) {
    return ON;
  }
  else {
    return OFF;
  }
}

bool statusDue() {
  return abs(millis() - lastStatusSendTime) > status_frequency;
}

bool validStation(int id) {
  return (id >= 1 && id <= STATIONS);
}

void turnAllStationsOff() {
  for (int station=1; station <= STATIONS; station++) {
    setStationRelayState(station, OFF);
  }
}
void setStationRelayState(int stationID, int state) {
  digitalWrite(stationRelays[stationID-1], state);
}

int getStationRelayState(int stationID) {
  return digitalRead(stationRelays[stationID-1]);
}

bool stationOn(int stationID) {
  return digitalRead(stationRelays[stationID-1]) == ON;
}

bool stationOff(int stationID) {
  return !stationOn(stationID);
}

bool isStatusDue() {
  currentTime = millis();
  return (abs(currentTime - lastStatusSendTime) > status_frequency);
}

char* topic(int stationID) {
    String topicString = "station/" + stationID;
    char topicArray[10];
    topicString.toCharArray(topicArray, 10);
    return topicArray;
}

char* stationStateMessage(int stationID) {
  char message[2];
  sprintf(message, "%d", getStationRelayState(stationID));
  return message;
}

