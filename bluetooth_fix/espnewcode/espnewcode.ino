// Libraries
#include <Arduino.h>
#include <string>
#include "BLEDevice.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncUDP.h>


// Definitions
#define SSID "VRADA" // name of the wifi access point
#define PASSWORD "vrada1234" // password of the wifi access point
#define MAX_CLIENTS 1
#define GENERAL_DEBUG // delete this line to disable general purpose debug
#define PACKETS_DEBUG // delete this line to disable packet transmit debug
#define STANDARD_DELAY 100 
#define SCAN_AT_LOOP_DELAY 1000
#define WIFI_DELAY 500 // time after each attempt to connect
#define BLE_SCAN_TIME 15 // time that the scanner will scan

// Global
WiFiUDP udp;
BLEScan* pBLEScan; // for the BLE scan 
static uint8_t trackedClients = 0;  // number of clients that we found and we will try to connect
static uint8_t connectedClients = 0;  // number of bluetooth clients that we are currently connected
static std::string PredefinedTOORXAddresses[] = {"00:0c:bf:26:c1:1d"};
static std::string PredefinedReceivers[] = {"2c:26:17:0d:3f:d7"};
static std::string deviceNames[] = {"TOORX0086"};
static uint8_t predefinedClients = 1;
static BLEAddress* TrackedClients[MAX_CLIENTS];

static BLEClient* ConnectedClients[MAX_CLIENTS];

//----------------------------------------------------------- Functions  -----------------------------------------------------------//

bool checkIfDevicesIsAlreadyTracked(BLEAdvertisedDevice advertisedDevice){
  for(int i=0;i<trackedClients;i++){
    if(advertisedDevice.getAddress().equals(*TrackedClients[i])){
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------  Classes  -----------------------------------------------------------// 
class AdvertisedToorxDeviceCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // If the device we tracked is a predefined TOORX and the max clients are not reached
    // then we include that device
    if(advertisedDevice.getName()!=""&& advertisedDevice.getName().find("TOORX") != std::string::npos 
      && trackedClients <= MAX_CLIENTS && !checkIfDevicesIsAlreadyTracked(advertisedDevice)
     ){
      for(int i=0;i<predefinedClients;i++){
        if (advertisedDevice.getAddress().equals(BLEAddress(PredefinedTOORXAddresses[i]))) {
          #ifdef GENERAL_DEBUG
            Serial.printf("[STATUS] Adding %s with address %s to the tracked devices.\n",advertisedDevice.getName().c_str(),PredefinedTOORXAddresses[i].c_str());
          #endif
          TrackedClients[trackedClients] = new BLEAddress(PredefinedTOORXAddresses[i]);
          trackedClients++;
        }
      }
    }
  }
};


//----------------------------------------------------------- Functions  -----------------------------------------------------------//

void setupWifi(){
  #ifdef GENERAL_DEBUG
    Serial.print("[STATUS] Trying to connect to the ~VRADA~ AP...");
  #endif
  // Connect to the wifi with ssid and password
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_DELAY);
    Serial.print(".");
  }
  #ifdef GENERAL_DEBUG
    Serial.println();
    Serial.println("[SUCCESS] Connected to the ~VRADA~ AP...");
  #endif
  
}
void scanForBLEDevices(){
  BLEDevice::init(""); // Start the bluetooth scanner
  
  // Scan for devices that are BLE and match the predefined ones
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedToorxDeviceCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(BLE_SCAN_TIME);
}

void connectWithTheTrackedDevices(){
  for(int i=0;i<trackedClients;i++){
    BLEClient*  tempClient  = BLEDevice::createClient();
    #ifdef GENERAL_DEBUG
      Serial.printf("[STATUS] Trying to connecting to %s\n", TrackedClients[i]->toString().c_str());
    #endif
    if (tempClient->connect(*TrackedClients[i])) {
      #ifdef GENERAL_DEBUG
        Serial.printf("[SUCCESS] Connected to %s\n",TrackedClients[i]->toString().c_str());
      #endif
      ConnectedClients[connectedClients] = tempClient;
      connectedClients++;
    } else{
      #ifdef GENERAL_DEBUG
        Serial.printf("[FAIL] Failed to connect to %s\n",TrackedClients[i]->toString().c_str());
      #endif
    }
  }
}

//-----------------------------------------------------------  Setup & Loop  -----------------------------------------------------------//
void setup() {
  Serial.begin(9600); // Start with 9600 speed
  #ifdef GENERAL_DEBUG
    delay(STANDARD_DELAY);
    Serial.println("[STATUS] Starting BLE client application....");
  #endif 
  
  setupWifi();
  
  #ifdef GENERAL_DEBUG
    Serial.println("[STATUS] Scanning for BLE Devices...");
  #endif
  scanForBLEDevices();
  delay(STANDARD_DELAY);

  #ifdef GENERAL_DEBUG
    Serial.println("[STATUS] Trying to connect with the tracked devices...");
  #endif
  connectWithTheTrackedDevices();
}

void loop() {
 
}
