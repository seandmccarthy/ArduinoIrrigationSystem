
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <JsonGenerator.h>
#include <JsonParser.h>

#define STATIONS 4

#define NAME_LENGTH 3
#define VALUE_LENGTH 2
#define MAX_PARAMS 2
#define PREFIX ""

#define ON 1
#define OFF 0

const unsigned long status_frequency = 60000; // every 60 seconds
unsigned long lastStatusSendTime;
unsigned long currentTime;

WebServer webserver(PREFIX, 80);
PubSubClient client(server, 1883, callback, ethClient);

byte stationRelays[STATIONS] =  {
  2, /* RELAY 1 */
  7, /* RELAY 2 */
  8, /* RELAY 3 */
  9  /* RELAY 4 */
};

static byte mac[] = { 0x5E, 0xED, 0x52, 0xFE, 0xED, 0x00 };
IPAddress host_ip(10, 0, 1, 201);

void stationCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int id;
  int state;
  
  getParametersFromRequest(id, state, server, &url_tail);
  if (!validStation(id)) {
    server.httpFail();
    return;
  }

  server.httpSuccess();
  if (hasState(state)) {
    setStationRelayState(id, state);
  }
  else {
    server.print(getStationRelayState(id)); 
  }
}

bool hasState(int state) {
  return state == OFF || state == ON;
}

void getParametersFromRequest(int& id, int& state, WebServer &server, char **url_tail) {
  URLPARAM_RESULT rc;
  char name[NAME_LENGTH];
  char value[VALUE_LENGTH];
  int maxParams = MAX_PARAMS;

  while (maxParams > 0) {
    rc = server.nextURLparam(url_tail, name, NAME_LENGTH, value, VALUE_LENGTH);
    if (rc == URLPARAM_EOS) {
      break;
    }
   
    if (strcmp(name, "id") == 0) {
      id = atoi(value);
    }
    else if (strcmp(name, "on") == 0) {
      state = atoi(value) == 1 ? ON : OFF;
    }
    maxParams--;
  }
}

bool validStation(int id) {
  return (id >= 1 && id <= STATIONS);
}

void turnAllStationsOff() {
  for (int station=1; station <= STATIONS; station++) {
    setStationRelayState(station, OFF);
  }
}

void setup() {
  Serial.begin(9600);
  for(int i = 0; i < STATIONS; i++) {
    pinMode(stationRelays[i], OUTPUT);
  }
  Ethernet.begin(mac, host_ip);
  webserver.setDefaultCommand(&stationCmd);
  webserver.addCommand("station", &stationCmd);
  webserver.begin();
  lastStatusSendTime = 0;
}

void loop() {
  while(true) {
    webserver.processConnection();
    if (isStatusDue()) {
      sendStatus();
    }
    delay(10);
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

void sendStatus() {
}

