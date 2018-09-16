#include  <Arduino.h>
#include  <ESP8266WiFi.h>
#include "fauxmoESP.h"

#define WIFI_SSID "..."
#define WIFI_PASS "..."

#define SERIAL_BAUDRATE 115200

#define DOWN  D1
#define UP    D2

struct State  // Struct to keep current state of device
{
  bool state = 0;
  const char *device_name;
  uint8_t device_id;
} Curstate;

fauxmoESP fauxmo;

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("Alexa <-> ESP demo sketch");
  Serial.println("After connection, ask Alexa/Echo to 'turn [DEVICE] on' or 'off'");

  wifiSetup();

  fauxmo.addDevice("Rollo Balkon"); // One or all blinds - work on multiple individual shades later
  Serial.println("Device 'Rollo Balkon' added");

  fauxmo.onMessage(callback);

  pinMode(DOWN, OUTPUT);
  pinMode(UP, OUTPUT);

  digitalWrite(DOWN, HIGH); // Set UP and DOWN HIGH so the remote doesn't activate on start up
  digitalWrite(UP, HIGH);
}

void loop() {
  static bool state;

  fauxmo.handle();

  if (state != Curstate.state)
  {
    state = Curstate.state;

    if (Curstate.state) {
      Serial.println("ON/DOWN");
      digitalWrite(DOWN, LOW);
      delay(5000);
      digitalWrite(DOWN, HIGH);
    } else {
      Serial.println("OFF/UP");
      digitalWrite(UP, LOW);
      delay(5000);
      digitalWrite(UP, HIGH);
    }
  }
}

void wifiSetup() {
  WiFi.mode(WIFI_STA);

  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void callback(uint8_t device_id, const char * device_name, bool state) {
  if (state == Curstate.state) return;

  Curstate.device_name = device_name;
  Curstate.state = state;
  Curstate.device_id = device_id;
}

