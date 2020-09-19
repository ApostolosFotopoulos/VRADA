// Libraries
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncUDP.h>
#include <string>

// Definitions
#define SSID "VRADA"
#define PASSWORD "vrada1234"
#define STANDARD_DELAY 200
#define PACKET_DELAY 800
#define DEBUG

// Global
AsyncUDP udpListener;

void connectToWifi(){
  WiFi.begin(SSID,PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(STANDARD_DELAY);
    Serial.print(".");
  }
  Serial.println();
  #ifdef DEBUG
    Serial.println("[STATUS] Successfully connected to the Wifi...");
  #endif
}
void udpPacketHandler(AsyncUDPPacket packet) {
  Serial.print("UDP Packet Type: ");
  Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
  Serial.print(", From: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.print(packet.remotePort());
  Serial.print(", To: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.print(packet.localPort());
  Serial.print(", Length: ");
  Serial.print(packet.length());
  Serial.print(", Data: ");
  Serial.write(packet.data(), packet.length());
  Serial.println();
  //reply to the client
  packet.printf("Got %u bytes of data", packet.length());
}
void setup(){
  
  // Start the serial
  Serial.begin(9600);
  delay(STANDARD_DELAY);
  while(!Serial);

  // Connect to wifi
  #ifdef DEBUG
    Serial.print("[STATUS] Trying to connect to the Wifi...");
  #endif
  connectToWifi();
   if (udpListener.listen(IPAddress(255, 255, 255, 255), 12346)) {
    Serial.println("UDP connected");
    udpListener.onPacket(udpPacketHandler);
    // Send unicast
  }
}


void loop(){
  
}
