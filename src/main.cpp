#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <LED.h>
#include <BlinkRoutines.h>

const uint8_t ipLength = 6;
uint8_t mac[ipLength];
uint8_t network_members[][ipLength] = {
    {0xEC, 0xFA, 0xBC, 0xC0, 0x9C, 0x35},
    {0x50, 0x02, 0x91, 0x68, 0xFA, 0x32}};

esp_now_role role = ESP_NOW_ROLE_SLAVE;
long last_msg_from_master = 0;
long startup_delay = 10000 + random(500);
LED leds;

typedef struct struct_message
{
  int mode;
} struct_message;

struct_message data;

uint8_t blinkRoutineIndex = 2;
BlinkRoutine *blinkRoutines[3];

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

void setupPeers(esp_now_role role)
{
  for (int i = 0; i < sizeof(network_members) / ipLength; i++)
  {
    esp_now_del_peer(network_members[i]);
    if (memcmp(network_members[i], mac, ipLength) != 0)
    {
      esp_now_add_peer(network_members[i], role, 1, NULL, 0);
    }
  }
}

void sendUpdateToPeers()
{
  for (int i = 0; i < sizeof(network_members) / ipLength; i++)
  {
    data.mode = blinkRoutineIndex;
    if (memcmp(network_members[i], mac, ipLength) != 0)
    {
      esp_now_send(network_members[i], (uint8_t *)&data, sizeof(data));
    }
  }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
#ifdef D_SERIAL
  D_SERIAL.print("Last Packet Send Status: ");
  if (sendStatus == 0)
    D_SERIAL.println("Delivery success");
  else
    D_SERIAL.println("Delivery fail");
#endif
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  last_msg_from_master = millis();
  memcpy(&data, incomingData, sizeof(data));
#ifdef D_SERIAL
  D_SERIAL.print("Bytes received: ");
  D_SERIAL.println(len);
  D_SERIAL.print("Int: ");
  D_SERIAL.println(data.mode);
#endif

  if (blinkRoutineIndex != data.mode)
  {
    blinkRoutineIndex = data.mode;
    leds.setRoutine(blinkRoutines[blinkRoutineIndex], last_msg_from_master);
  }
  else
  {
    leds.synchronize(last_msg_from_master);
  }
  if (role == ESP_NOW_ROLE_CONTROLLER)
  {
#ifdef D_SERIAL
    D_SERIAL.println("Master found. Becoming slave!");
#endif
    role = ESP_NOW_ROLE_SLAVE;
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    setupPeers(ESP_NOW_ROLE_CONTROLLER);
  }
}

void setup()
{
#ifdef D_SERIAL
  D_SERIAL.begin(115200);
  delay(2000);
  D_SERIAL.println();
  D_SERIAL.print("ESP8266 Board MAC Address:  ");
  D_SERIAL.println(WiFi.macAddress());
#endif

  blinkRoutines[0] = new SingleBlinkRoutine(10, 300);
  blinkRoutines[1] = new DoubleBlinkRoutine();
  blinkRoutines[2] = new FadeRoutine(500);
  leds.setRoutine(blinkRoutines[blinkRoutineIndex], 0);
  WiFi.macAddress(mac);
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
#ifdef D_SERIAL
    D_SERIAL.println("Error initializing ESP-NOW");
#endif
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  setupPeers(ESP_NOW_ROLE_CONTROLLER);
}

void loop()
{
  long t = millis();
  if (role == ESP_NOW_ROLE_CONTROLLER)
  {
    if ((t - lastTime) >= timerDelay)
    {
      sendUpdateToPeers();
      lastTime = t;
      timerDelay = leds.setRoutine(blinkRoutines[blinkRoutineIndex], t);
    }
  }
  leds.loop(t);
  if (last_msg_from_master + startup_delay < t && role == ESP_NOW_ROLE_SLAVE)
  {
#ifdef D_SERIAL
    D_SERIAL.println("No master. Taking over!");
#endif
    role = ESP_NOW_ROLE_CONTROLLER;
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    setupPeers(ESP_NOW_ROLE_SLAVE);
  }
}