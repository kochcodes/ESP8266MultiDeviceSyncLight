#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>
#include <LED.h>
#include <BlinkRoutines.h>

const uint8_t ipLength = 6;
uint8_t mac[ipLength];
uint8_t network_members[][ipLength] = {
    {0x5C, 0xCF, 0x7F, 0xAC, 0xBD, 0xFC}, // 5C:CF:7F:AC:BD:FC connector
    {0x98, 0xF4, 0xAB, 0xDA, 0xB3, 0x70}, // 98:F4:AB:DA:B3:70 member
    {0xEC, 0xFA, 0xBC, 0xC0, 0x9C, 0x35}, // EC:FA:BC:C0:9C:35 member
    {0x98, 0xF4, 0xAB, 0xDA, 0xB8, 0x26}, // 98:F4:AB:DA:B8:26 member
    {0x30, 0xAE, 0xA4, 0x8D, 0xE7, 0x2C}, // 30:AE:A4:8D:E7:2C esp32 connector
    {0x30, 0xAE, 0xA4, 0x8D, 0xE7, 0x2D}  // 30:AE:A4:8D:E7:2D esp32 connector
};

esp_now_role role = ESP_NOW_ROLE_SLAVE;
long last_msg_from_master = 0;
long startup_delay = 10000 + random(500);
LED leds;

typedef struct struct_message
{
  int mode;
} struct_message;

struct_message data;

const uint8_t blinkRoutinesNumber = 4;
uint8_t blinkRoutineIndex = 1; // will be owerwritten by value stored in eeprom
uint8_t new_mode_to_save = blinkRoutineIndex;
BlinkRoutine *
    blinkRoutines[blinkRoutinesNumber];

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

void init_eeprom()
{
  EEPROM.begin(512);
}

void save_mode_to_eeprom(uint8_t mode)
{
  int mode_addr = 0;
  EEPROM.put(mode_addr, mode);
  EEPROM.commit();
}

int get_mode_from_eeprom()
{
  int mode_addr = 0;
  uint8_t mode = 0;
  EEPROM.get(mode_addr, mode);
  return mode;
}

void setupPeers(esp_now_role role)
{
  for (unsigned int i = 0; i < sizeof(network_members) / ipLength; i++)
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
  for (unsigned int i = 0; i < sizeof(network_members) / ipLength; i++)
  {
    data.mode = blinkRoutineIndex;
    if (memcmp(network_members[i], mac, ipLength) != 0)
    {
#ifdef D_SERIAL
      D_SERIAL.printf("Sending to member %d\n", i);
      D_SERIAL.printf("Blink-Mode: %d\n", blinkRoutineIndex);
#endif
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

  D_SERIAL.print("Bytes received: ");
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
    save_mode_to_eeprom(blinkRoutineIndex);
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

  init_eeprom();
  blinkRoutineIndex = get_mode_from_eeprom();
  new_mode_to_save = (blinkRoutineIndex + 1) % blinkRoutinesNumber;
  save_mode_to_eeprom(new_mode_to_save);

#ifdef D_SERIAL
  D_SERIAL.print("Blink mode from EEPROM:  ");
  D_SERIAL.println(blinkRoutineIndex);
#endif

  blinkRoutines[0] = new SingleBlinkRoutine(10, 300);
  blinkRoutines[1] = new DoubleBlinkRoutine();
  blinkRoutines[2] = new FadeRoutine(2000);
  blinkRoutines[3] = new DoubleBlinkSemiRoutine();
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
  if (startup_delay < t && new_mode_to_save != blinkRoutineIndex)
  {
#ifdef D_SERIAL
    D_SERIAL.print("Reset blinkmode to ");
    D_SERIAL.println(blinkRoutineIndex);
#endif
    new_mode_to_save = blinkRoutineIndex;
    save_mode_to_eeprom(blinkRoutineIndex);
  }

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