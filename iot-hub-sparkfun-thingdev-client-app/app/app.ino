#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

#include "config.h"

static bool messagePending = false;
static bool messageSending = false;

static char *connectionString;
static char *ssid;
static char *pass;

static int interval = INTERVAL;

void blinkLED()
{
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

void initWifi()
{
  Serial.printf("Attempting to connect to SSID: %s.\r\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.printf("Connected to wifi %s.\r\n", ssid);
}

void initTime()
{
  time_t epochTime;
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  while (true)
  {
    epochTime = time(NULL);

    if (epochTime == 0)
    {
      Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
      delay(2000);
    }
    else
    {
      Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
      break;
    }
  }
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  initSerial();
  delay(2000);
  readCredentials();

  initWifi();
  initTime();
  initSensor();

  /*
    Break changes in version 1.0.34: AzureIoTHub library removed AzureIoTClient class.
    So we remove the code below to avoid compile error.
  */
  // initIoThubClient();

  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
  if (iotHubClientHandle == NULL)
  {
    Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
    while (1);
  }

  IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
  IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
  IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
}

static int messageCount = 1;

void loop()
{
  if (!messagePending && messageSending)
  {
    char messagePayload[MESSAGE_MAX_LEN];
    bool temperatureAlert = readMessage(messageCount, messagePayload);

    sendMessage(iotHubClientHandle, messagePayload, temperatureAlert);

    messageCount++;
    delay(interval);
  }

  IoTHubClient_LL_DoWork(iotHubClientHandle);
  delay(10);
}

