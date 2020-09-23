// Libraries
#include <Arduino.h>
#include <string>
#include "BLEDevice.h"
#include <WiFi.h>


//Definitions
#define SSID "nitlab-2.4G-2os"
#define PASSWORD "nitlabuth"
#define MAX_CLIENTS 2
#define DEBUG
#define STANDARD_DELAY 400
#define BLE_SCAN_TIME 10

//Global
WiFiUDP udp;
BLEScan *pBLEScan;
static uint8_t trackedClients = 0; // number of clients that we found and try to connect
static uint8_t connectedClients = 0; // number of bluetooth clients that we are connected to
static std::string PredefinedTOORXAddresses[] = {"00:0c:bf:26:c1:1d","a4:5e:60:dc:21:7a"};
static std::string PredefinedReceivers[] = {"2c:26:17:0d:3f:d7"};
static std::string deviceNames[] = {"TOORX0086","TOORX0087"};
static uint8_t predefinedClients = 2; 
static BLEAddress* TrackedClients[MAX_CLIENTS]; // Addresses that we tracked
static BLEClient* ConnectedClients[MAX_CLIENTS]; // Addresses of the client that connect

// UUIDs and characteristics
static std::string serviceUUIDs[] = {"49535343-fe7d-4ae5-8fa9-9fafd205e455","49535343-fe7d-4ae5-8fa9-9fafd205e456"};
static std::string writeUUIDs[] = {"49535343-8841-43f4-a8d4-ecbe34729bb3","49535343-8841-43f4-a8d4-ecbe34729bb4"};
static std::string readUUIDs[] = {"49535343-1e4d-4bd9-ba61-23c647249616","49535343-1e4d-4bd9-ba61-23c647249617"};
static std::string notificationDescUUIDs[] = {"0x2902"};

// Notification on and off values
const uint8_t notificationsOn[] = {0x1, 0x0}; 
const uint8_t notificationsOff[] = {0x0, 0x0};


// For the bike 
uint8_t ping1[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t ping2[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t start[] = {0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb};
uint8_t details[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};

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
int mapTrackedToPredifinedIndex(int idx){
  for(int i=0;i<predefinedClients;i++){
    if(TrackedClients[idx]->toString() == PredefinedTOORXAddresses[i]){
      return i;
    }
  }
  return -1;
}
void connectClients(){
  for(int i=0;i<trackedClients;i++){
    ConnectedClients[connectedClients] = BLEDevice::createClient();
    #ifdef DEBUG
      Serial.printf("[STATUS] Trying to connecting to %s\n", TrackedClients[i]->toString().c_str());
    #endif
    if (ConnectedClients[connectedClients]->connect(*TrackedClients[i])) {
      #ifdef DEBUG
        Serial.printf("[SUCCESS] Connected to %s\n",TrackedClients[i]->toString().c_str());
      #endif
      if(ConnectedClients[i]->toString() == "TOORX0086"){
        int idx = mapTrackedToPredifinedIndex(i);
        Serial.println(serviceUUIDs[idx].c_str());

        Serial.println("[STATUS] PING1 Signal");
        ConnectedClients[connectedClients]
            ->getService(BLEUUID(serviceUUIDs[idx]))
            ->getCharacteristic(BLEUUID(writeUUIDs[idx]))
            ->writeValue(ping1, sizeof(ping1), true);
        delay(STANDARD_DELAY);
      
        Serial.println("[STATUS] PING2 Signal");
        ConnectedClients[connectedClients]
            ->getService(BLEUUID(serviceUUIDs[idx]))
            ->getCharacteristic(BLEUUID(writeUUIDs[idx]))
            ->writeValue(ping2, sizeof(ping2), true);
        delay(STANDARD_DELAY);
        
        Serial.println("[STATUS] START Signal");
        ConnectedClients[connectedClients]
            ->getService(BLEUUID(serviceUUIDs[idx]))
            ->getCharacteristic(BLEUUID(writeUUIDs[idx]))
            ->writeValue(start, sizeof(start), true);
        delay(STANDARD_DELAY);
      
      } else {
        int idx = mapTrackedToPredifinedIndex(i);
        Serial.println(serviceUUIDs[idx].c_str());
        uint8_t val[] = {1};
        ConnectedClients[connectedClients]
          ->getService(BLEUUID(serviceUUIDs[idx]))
          ->getCharacteristic(BLEUUID(writeUUIDs[idx]))
          ->writeValue(val,1, true);
      }
      
      connectedClients++; // last line
    } else {
      #ifdef DEBUG
        Serial.printf("[FAIL] %s failed to connect.\n",TrackedClients[i]->toString().c_str());
      #endif
    }
    delay(STANDARD_DELAY);
  }
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

  // Connect to the tracked devices
  connectClients();
}

int counter = 2;
void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<connectedClients;i++){
    if(ConnectedClients[i]->toString() == "TOORX0086"){
        int idx = mapTrackedToPredifinedIndex(i);
        ConnectedClients[i]->getService(BLEUUID(serviceUUIDs[idx]))->getCharacteristic(BLEUUID(writeUUIDs[idx]))->writeValue(details, sizeof(details), true);
      } else {
        int idx = mapTrackedToPredifinedIndex(i);
        
        uint8_t val[] = {counter};
        Serial.printf("Writing to the virtual: %d\n",counter);
        ConnectedClients[i]->getService(BLEUUID(serviceUUIDs[idx]))->getCharacteristic(BLEUUID(writeUUIDs[idx]))->writeValue(val,1, true);
        counter++;
      }
  }
  delay(800);
}
