// Libraries
#include <Arduino.h>
#include <string>
#include "BLEDevice.h"
#include <WiFi.h>


//Definitions
#define SSID "VRADA"
#define PASSWORD "vrada1234"
#define MAX_CLIENTS 2
#define DEBUG
#define STANDARD_DELAY 200

//Global
WiFiUDP udp;
BLEScan *pBLEScan;
static uint8_t trackedClients = 0; // number of clients that we found and try to connect
static uint8_t connectedClients = 0; // number of bluetooth clients that we are connected to


//-------------------------------------------------------------------- Functions -------------------------------------------//

void setupWiFi(){
  #ifdef DEBUG
    Serial.print("Trying to connect to the ~VRADA~ AP...");
  #endif 
  WiFi.begin(SSID,PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(STANDARD_DELAY);
    Serial.print(".");
  }
  #ifdef DEBUG
    Serial.println();
    Serial.println("Connected to the ~VRADA~ AP...");
  #endif
}
void setup() {
  Serial.begin(9600); // Start at frequency 96000
  delay(STANDARD_DELAY);
  while(!Serial);

  #ifdef DEBUG
    Serial.println("Starting the BLE client application...");
  #endif

  // Connect to the WiFi
  setupWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:

}
