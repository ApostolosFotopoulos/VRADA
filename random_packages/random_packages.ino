#include <WiFi.h>
#include <WiFiUdp.h>

#include <AsyncUDP.h>

#define MAX_CLIENTS 8

#define SSID "VRADA"   // name of the WiFi AP
#define PASSWORD "vrada1234"  // password of the WiFi AP
#define DEBUG

WiFiUDP udp;

void generateRandomAndSendPacket(){
  // Prepare the buffer for the packet
  String bufferStr;
  int r;
  bufferStr.concat("VRADA;");
  bufferStr.concat("00:0c:bf:26:c1:1d");
  bufferStr.concat(";speed:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";rpm:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";distance:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";calories:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";hf:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";power:");
  r = random(1,10);
  bufferStr.concat((100*r)/10.0);
  bufferStr.concat(";level:");
  bufferStr.concat(1);
  uint8_t* buffer = (uint8_t*)reinterpret_cast<const uint8_t*>(bufferStr.c_str());
  #ifdef DEBUG
    printf("%s\n", buffer);
  #endif
  SendPacket(buffer, bufferStr.length());
}
void SendPacket(uint8_t* data, size_t length) {
  udp.beginPacket("255.255.255.255", 12345);
  udp.write(data, length);
  udp.endPacket();
}
void setup() {
  Serial.begin(9600);
  #ifdef DEBUG
    Serial.println("Starting Arduino BLE Client application...");
  #endif
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  #ifdef DEBUG
    Serial.println("Authentication with the Wifi is done...");
  #endif
}

void loop() {  
  Serial.println("Generating random packet....");
  generateRandomAndSendPacket();
  delay(400);
}
