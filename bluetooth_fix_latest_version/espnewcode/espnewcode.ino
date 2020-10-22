// Libraries
#include <Arduino.h>
#include <string>
#include "BLEDevice.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AsyncUDP.h>


// Definitions
int status = WL_IDLE_STATUS;
#define SSID "VRADA" // name of the wifi access point
#define PASSWORD "Password123" // password of the wifi access point
#define MAX_CLIENTS 1
#define GENERAL_DEBUG // delete this line to disable general purpose debug
#define PACKETS_DEBUG // delete this line to disable packet transmit debug
#define NOTIFY_DEBUG 
#define STANDARD_DELAY 100 
#define LOOP_DELAY 400
#define WIFI_DELAY 500 // time after each attempt to connect
#define BLE_SCAN_TIME 5// time that the scanner will scan

// Global
WiFiUDP udp;
BLEScan* pBLEScan; // for the BLE scan 
static uint8_t trackedClients = 0;  // number of clients that we found and we will try to connect
static uint8_t connectedClients = 0;  // number of bluetooth clients that we are currently connected
static std::string PredefinedTOORXAddresses[] = {"00:0c:bf:13:e4:a4"};
static std::string PredefinedReceivers[] = {"2c:26:17:11:fa:17"};
static std::string deviceNames[] = {"TOORX0001"};
static uint8_t predefinedClients = 1; 
static BLEAddress* TrackedClients[MAX_CLIENTS]; // Addresses that we tracked
static BLEClient* ConnectedClients[MAX_CLIENTS]; // Addresses of the client that connect

// UUIDs and characteristics
static BLEUUID serviceUUID("49535343-fe7d-4ae5-8fa9-9fafd205e455");
static BLEUUID writeUUID("49535343-8841-43f4-a8d4-ecbe34729bb3");
static BLEUUID readUUID("49535343-1e4d-4bd9-ba61-23c647249616");
static BLEUUID notificationsDescrUUID(BLEUUID((uint16_t)0x2902)); // Descriptor of the notifications

// Notification on and off values
const uint8_t notificationsOn[] = {0x1, 0x0}; 
const uint8_t notificationsOff[] = {0x0, 0x0};

uint8_t ping1[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t ping2[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t start[] = {0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb};
uint8_t details[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};

AsyncUDP udpListener;


//----------------------------------------------------------- Functions  -----------------------------------------------------------//
void ChangeLevel(std::string sender, uint8_t level);

void udpPacketHandler(AsyncUDPPacket packet) {
  Serial.println("Inside the handler");
  std::string packetData = std::string((char*)packet.data());
  if (packetData.length() < 5) {
    return;
  }
  if (packetData.substr(0, 6) != "VRADA;") {
    #ifdef PACKETS_DEBUG
      Serial.println("Not VRADA packet");
    #endif
    return;
  }
  #ifdef PACKETS_DEBUG
    Serial.printf("Packet length: %d and packetData[23] = %c\n",
                  packetData.length(), packetData[23]);
  #endif
  if (packetData.length() > 23 && packetData[23] != ';') {
    #ifdef PACKETS_DEBUG
      Serial.println("Expected to find MAC Address length string. Exiting");
    #endif
    return;
  }
  std::string mac = packetData.substr(6, 17);
  std::string tempKey = "";    // we store the string till a = is found
                               // we found an attribute
  std::string tempValue = "";  // we store here the string till a ; is found
  bool foundKey = false;
  for (int i = 24; i < packetData.length(); i++) {
    if (!foundKey) {
      if (packetData[i] == '=') {
        foundKey = true;
        #ifdef PACKETS_DEBUG
          Serial.printf("Found key: %s\n", tempKey.c_str());
        #endif
      } else {
        tempKey += packetData[i];
      }
    } else {
      // check if we have a ; or the end of string without ;
      if (packetData[i] == ';' || (i + 1) == packetData.length()) {
        if ((i + 1) == packetData.length() && packetData[i] != ';') {
          tempValue += packetData[i];
        }
        foundKey = false;
        #ifdef PACKETS_DEBUG 
          Serial.printf("Key : %s, value : %s, mac : %s\n", tempKey.c_str(),
                              tempValue.c_str(), mac.c_str());
        #endif
        // here we execute the command to set the value
        if (tempKey == "level") {
          ChangeLevel(mac, (uint8_t)atoi(tempValue.c_str()));
          delay(STANDARD_DELAY);
        }
        tempKey = "";
        tempValue = "";
      } else {
        tempValue += packetData[i];
      }
    }
  }
}

void SendPacket(uint8_t* data, size_t length) {
  udp.beginPacket("255.255.255.255", 12345);
  udp.write(data, length);
  udp.endPacket();
}

void ChangeLevel(std::string sender, uint8_t level) {
  transform(sender.begin(), sender.end(), sender.begin(), ::tolower);
  std::string address = "";
  for (int i = 0; i < predefinedClients; i++) {
    transform(PredefinedReceivers[i].begin(), PredefinedReceivers[i].end(),
              PredefinedReceivers[i].begin(), ::tolower);
    if (PredefinedReceivers[i] == sender) {
      address = PredefinedTOORXAddresses[i];
      break;
    }
  }
  if (address == "") {
    return;
  }
  #ifdef GENERAL_DEBUG
    Serial.printf("[Change level] Setting level to %d\n", level);
  #endif
  uint8_t levelPacket[] = {
      0xf0,
      0xa6,
      0x01,
      0x01,
      (uint8_t)(level + (uint8_t)1),
      (uint8_t)((uint8_t)(0xf0 + 0xa6 + 0x3 + level) & (uint8_t)0xff)};

  for (int i = 0; i < connectedClients; i++) {
    if (ConnectedClients[i]->getPeerAddress().equals(BLEAddress(address))) {
      ConnectedClients[i]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(levelPacket, sizeof(levelPacket), true);
      delay(STANDARD_DELAY);
      return;
    }
  }
}


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
void TOORXDataNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                             uint8_t* data, size_t length, bool isNotify) {
  std::string deviceName;
  std::string receiver;
  #ifdef NOTIFY_DEBUG
    Serial.print("Received reply from ");
  #endif
  // find device name
  for (int i = 0; i < trackedClients; i++) {
    if (pBLERemoteCharacteristic->getRemoteService()
            ->getClient()
            ->getPeerAddress()
            .toString() == TrackedClients[i]->toString()) {
      deviceName = deviceNames[i];
      #ifdef NOTIFY_DEBUG
        Serial.print(deviceNames[i].c_str());
      #endif
    }
  }
  // find who needs to receive for this device
  for (int i = 0; i < predefinedClients; i++) {
    if (pBLERemoteCharacteristic->getRemoteService()
            ->getClient()
            ->getPeerAddress()
            .equals(BLEAddress(PredefinedTOORXAddresses[i]))) {
      receiver = PredefinedReceivers[i];
    }
  }
  #ifdef NOTIFY_DEBUG
    Serial.print(" : ");
    for (int i = 0; i < length; i++) {
      Serial.printf("%x", (int)data[i]);
    }
  #endif
  if (length > 11) {
    String bufferStr;
    double speed = (100 * ((int)data[6] - 1) + ((int)data[7] - 1)) / 10.0;
    int rpm = 100 * ((int)data[8] - 1) + (data[7] - 1);
    double distance = (100 * (data[10] - 1) + data[11] - 1) / 10.0;
    int calories = (100 * (data[12] - 1) + data[13] - 1);
    int hf = (100 * (data[14] - 1) + data[15] - 1);
    double power = (100 * (data[16] - 1) + data[17] - 1) / 10.0;
    int level = data[18] - 1;
    bufferStr.concat("VRADA;");
    bufferStr.concat(receiver.c_str());
    bufferStr.concat(";speed:");
    bufferStr.concat(speed);
    bufferStr.concat(";rpm:");
    bufferStr.concat(rpm);
    bufferStr.concat(";distance:");
    bufferStr.concat(distance);
    bufferStr.concat(";calories:");
    bufferStr.concat(calories);
    bufferStr.concat(";hf:");
    bufferStr.concat(hf);
    bufferStr.concat(";power:");
    bufferStr.concat(power);
    bufferStr.concat(";level:");
    bufferStr.concat(level);
    uint8_t* buffer =
        (uint8_t*)reinterpret_cast<const uint8_t*>(bufferStr.c_str());
    #ifdef NOTIFY_DEBUG
        printf("\n%s\n", buffer);
    #endif
    SendPacket(buffer, bufferStr.length());
  }
}

void setupWifi(){
  #ifdef GENERAL_DEBUG
    Serial.print("[STATUS] Trying to connect to the ~VRADA~ AP...");
  #endif
  // Connect to the wifi with ssid and password
  while (status != WL_CONNECTED) {
    status = WiFi.begin(SSID, PASSWORD);
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
    ConnectedClients[connectedClients] = BLEDevice::createClient();
    #ifdef GENERAL_DEBUG
      Serial.printf("[STATUS] Trying to connecting to %s\n", TrackedClients[i]->toString().c_str());
    #endif
    if (ConnectedClients[connectedClients]->connect(*TrackedClients[i])) {
      #ifdef GENERAL_DEBUG
        Serial.printf("[SUCCESS] Connected to %s\n",TrackedClients[i]->toString().c_str());
      #endif

       // Setup the client to read and notify us
      #ifdef GENERAL_DEBUG
        Serial.println("[STATUS] Trying to setup the notify callback");
      #endif
      ConnectedClients[connectedClients]->getService(serviceUUID)->getCharacteristic(readUUID)->registerForNotify(TOORXDataNotifyCallback);
      
      //Enabling notifications
      #ifdef GENERAL_DEBUG
        Serial.println("[STATUS] Enabling notifications");
      #endif
      ConnectedClients[connectedClients]->getService(serviceUUID)->getCharacteristic(readUUID)
        ->getDescriptor(notificationsDescrUUID)->writeValue((uint8_t*)notificationsOn,2,true);

      Serial.println("[STATUS] PING1 Signal");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(ping1, sizeof(ping1), true);
      delay(STANDARD_DELAY);
      Serial.println("[STATUS] PING2 Signal");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(ping2, sizeof(ping2), true);
      delay(STANDARD_DELAY);
      Serial.println("[STATUS] START Signal");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(start, sizeof(start), true);
      delay(STANDARD_DELAY);
      connectedClients++; // last line
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
  //while (!Serial);
  
  #ifdef GENERAL_DEBUG
    delay(STANDARD_DELAY);
    Serial.println("[STATUS] Starting BLE client application....");
  #endif 
  
  setupWifi();
  
  #ifdef GENERAL_DEBUG
    Serial.println("[STATUS] Scanning for BLE Devices...");
  #endif
  scanForBLEDevices();

  // Setup the udp handler
  if (udpListener.listen(IPAddress(255, 255, 255, 255), 12346)) {
    Serial.println("UDP connected");
    udpListener.onPacket(udpPacketHandler);
  }

  #ifdef GENERAL_DEBUG
    Serial.println("[STATUS] Trying to connect with the tracked devices...");
  #endif
  connectWithTheTrackedDevices();
}

void loop() {
 for(int i=0;i<connectedClients;i++){
  if(!ConnectedClients[i]->isConnected()){
    continue;
  } else {
    ConnectedClients[i]
        ->getService(serviceUUID)
        ->getCharacteristic(writeUUID)
        ->writeValue(details, sizeof(details), true);
  } 
 }
}
