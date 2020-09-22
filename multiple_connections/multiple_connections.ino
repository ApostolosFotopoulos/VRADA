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
#define STANDARD_DELAY 400
#define BLE_SCAN_TIME 10

//Global
WiFiUDP udp;
BLEScan *pBLEScan;
static uint8_t trackedClients = 0; // number of clients that we found and try to connect
static uint8_t connectedClients = 0; // number of bluetooth clients that we are connected to
static std::string PredefinedTOORXAddresses[] = {"00:0c:bf:26:c1:1d","7b:06:fb:b9:7c:2c"};
static std::string PredefinedReceivers[] = {"2c:26:17:0d:3f:d7"};
static std::string deviceNames[] = {"TOORX0086","Galaxy A50"};
static uint8_t predefinedClients = 2; 
static BLEAddress* TrackedClients[MAX_CLIENTS]; // Addresses that we tracked
static BLEClient* ConnectedClients[MAX_CLIENTS]; // Addresses of the client that connect

//-------------------------------------------------------------------- Functions -------------------------------------------//
bool checkIfDevicesIsAlreadyTracked(BLEAdvertisedDevice advertisedDevice){
  for(int i=0;i<trackedClients;i++){
    if(advertisedDevice.getAddress().equals(*TrackedClients[i])){
      return true;
    }
  }
  return false;
}
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

//-------------------------------------------------------------------- Classes -------------------------------------------//
class AdvertisedToorxDeviceCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // If the device we tracked is a predefined TOORX and the max clients are not reached
    // then we include that device
    Serial.println(advertisedDevice.getName().c_str());
    Serial.println(advertisedDevice.getAddress().toString().c_str());
    if(trackedClients <= MAX_CLIENTS && !checkIfDevicesIsAlreadyTracked(advertisedDevice)){
      for(int i=0;i<predefinedClients;i++){
        if (advertisedDevice.getAddress().equals(BLEAddress(PredefinedTOORXAddresses[i]))) {
          #ifdef DEBUG
            Serial.printf("Adding %s with address %s to the tracked devices.\n",advertisedDevice.getName().c_str(),PredefinedTOORXAddresses[i].c_str());
          #endif
          TrackedClients[trackedClients] = new BLEAddress(PredefinedTOORXAddresses[i]);
          trackedClients++;
        }
      }
    }
  }
};

void scanForBLEDevices(){
  BLEDevice::init(""); // Start the bluetooth scanner
  
  // Scan for devices that are BLE and match the predefined ones
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedToorxDeviceCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(BLE_SCAN_TIME);
}
void setup() {
  Serial.begin(9600); // Start at frequency 96000
  while(!Serial);
  delay(STANDARD_DELAY);

  #ifdef DEBUG
    Serial.println("\nStarting the BLE client application...");
  #endif

  // Connect to the WiFi
  setupWiFi();
  delay(STANDARD_DELAY);

  // Track BLE devices
  #ifdef DEBUG
    Serial.println("Scanning for BLE Devices...");
  #endif
  scanForBLEDevices();
}

void loop() {
  // put your main code here, to run repeatedly:

}
