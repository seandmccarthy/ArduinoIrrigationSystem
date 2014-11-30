
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <JsonGenerator.h>
#include <JsonParser.h>

#define STATIONS 4
#define PREFIX ""

WebServer webserver(PREFIX, 80);

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
  bool setState = false;
  int state;
  
  getParameters(server, &url_tail, id, state); 

  server.httpSuccess();
  if (validState(state)) {
    setStationRelayState(id, state);
  }
  else {
    server.print(getStationRelayState(id)); 
  }
}

bool validState(int state) {
  return state == 0 || state == 1;
}

void getParameters(WebServer &server, char **url_tail, int& id, int& state) {
  URLPARAM_RESULT rc;
  char name[3], value[2];
  for (;;) {
    rc = server.nextURLparam(url_tail, name, 3, value, 2);
    if (rc == URLPARAM_EOS) {
      break;
    }
   
    if (strcmp(name, "id") == 0) {
      id = atoi(value);
      if (id < 1 || id > STATIONS) {
        return;
      }
    }
    else if (strcmp(name, "on") == 0) {
      state = atoi(value);
      if (state > 1) {
        state = 1;
      }
    }
  }
}

void setup() {
  for(int i = 0; i < STATIONS; i++) {
    pinMode(stationRelays[i], OUTPUT);
  }
  Ethernet.begin(mac, host_ip);
  webserver.setDefaultCommand(&stationCmd);
  webserver.addCommand("station", &stationCmd);
  webserver.begin();
}

void loop() {
  while(true) {
    webserver.processConnection();
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
  return digitalRead(stationRelays[stationID-1]) == 1;
}

bool stationOff(int stationID) {
  return !stationOn(stationID);
}


