#include <Arduino.h>

#include <string>

#include "BLEDevice.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#include <AsyncUDP.h>

#define MAX_CLIENTS 8

#define SSID "Spanagiot - Guest"   // name of the WiFi AP
#define PASSWORD "spanagiotguest"  // password of the WiFi AP

#define DEBUG  // delete this line to disable debug serial prints

static std::string PredefinedTOORXAddresses[] = {"00:0c:bf:26:c1:1d"};
static std::string PredefinedReceivers[] = {"2c:26:17:0d:3f:d7"};
static std::string deviceNames[] = {"TOORX0086"};
static uint8_t predefinedClients = 1;

WiFiUDP udp;

static BLEAddress* TOORXAddresses[MAX_CLIENTS];

static BLEClient* ConnectedClients[MAX_CLIENTS];
static uint8_t currentClients =
    0;  // number of clients that we found and we will try to connect
static uint8_t connectedClients =
    0;  // number of bluetooth clients that we are currently connected

// The remote service we wish to connect to.
static BLEUUID serviceUUID("49535343-fe7d-4ae5-8fa9-9fafd205e455");
// The characteristic of the remote service we are interested in.
static BLEUUID writeUUID("49535343-8841-43f4-a8d4-ecbe34729bb3");
static BLEUUID readUUID("49535343-1e4d-4bd9-ba61-23c647249616");
uint8_t ping1[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t ping2[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
uint8_t start[] = {0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb};
uint8_t details[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};

AsyncUDP udpListener;

void ChangeLevel(std::string sender, uint8_t level);

void udpPacketHandler(AsyncUDPPacket packet) {
  std::string packetData = std::string((char*)packet.data());
  if (packetData.length() < 5) {
    return;
  }
  if (packetData.substr(0, 6) != "VRADA;") {
#ifdef DEBUG
    Serial.println("Not VRADA packet");
#endif
    return;
  }
#ifdef DEBUG
  Serial.printf("Packet length: %d and packetData[23] = %c\n",
                packetData.length(), packetData[23]);
#endif
  if (packetData.length() > 23 && packetData[23] != ';') {
#ifdef DEBUG
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
#ifdef DEBUG
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
#ifdef DEBUG
        Serial.printf("Key : %s, value : %s, mac : %s\n", tempKey.c_str(),
                      tempValue.c_str(), mac.c_str());
#endif
        // here we execute the command to set the value
        if (tempKey == "level") {
          ChangeLevel(mac, (uint8_t)atoi(tempValue.c_str()));
          delay(100);
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
#ifdef DEBUG
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
      delay(100);
      return;
    }
  }
}

void TOORXDataNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                             uint8_t* data, size_t length, bool isNotify) {
  std::string deviceName;
  std::string receiver;
#ifdef DEBUG
  Serial.print("Received reply from ");
#endif
  // find device name
  for (int i = 0; i < currentClients; i++) {
    if (pBLERemoteCharacteristic->getRemoteService()
            ->getClient()
            ->getPeerAddress()
            .toString() == TOORXAddresses[i]->toString()) {
      deviceName = deviceNames[i];
#ifdef DEBUG
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
#ifdef DEBUG
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
#ifdef DEBUG
    printf("\n%s\n", buffer);
#endif
    SendPacket(buffer, bufferStr.length());
  }
}

// Class that is needed for BLE Scanner
class AdvertisedToorxDeviceCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName().find("TOORX") != std::string::npos &&
        currentClients <= MAX_CLIENTS) {
      Serial.println("Found Toorx!");

      // find if the address we see belongs to predefined address
      for (int i = 0; i < predefinedClients; i++) {
        if (advertisedDevice.getAddress().equals(
                BLEAddress(PredefinedTOORXAddresses[i]))) {
          Serial.printf("Adding %s to TOORXAddresses\n",
                        PredefinedTOORXAddresses[i].c_str());
          TOORXAddresses[currentClients] =
              new BLEAddress(PredefinedTOORXAddresses[i]);
        }
      }
      Serial.print("Saving the name ");
      Serial.print(advertisedDevice.getName().c_str());
      Serial.println();
      deviceNames[currentClients] = advertisedDevice.getName();
      Serial.print("Name saved as ");
      Serial.println(deviceNames[currentClients].c_str());
      currentClients++;
    }
  }
};

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
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when
  // we have detected a new device.  Specify that we want active scanning and
  // start the scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedToorxDeviceCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5);

  if (udpListener.listen(IPAddress(255, 255, 255, 255), 12346)) {
    Serial.println("UDP connected");
    udpListener.onPacket(udpPacketHandler);
    // Send unicast
  }
  // When a TOORX device is found and there is space, we create a new client
  for (int i = 0; i < currentClients; i++) {
    BLEClient* tempClient = BLEDevice::createClient();
    Serial.printf("Connecting to %s\n", TOORXAddresses[i]->toString().c_str());
    if (tempClient->connect(*TOORXAddresses[i])) {
      Serial.println("Connected!");
      ConnectedClients[connectedClients] = tempClient;
      Serial.println("Registering callback");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(readUUID)
          ->registerForNotify(TOORXDataNotifyCallback);
      Serial.println("Turning on notifications");
      const uint8_t notificationsOn[] = {0x1, 0x0};
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(readUUID)
          ->getDescriptor(BLEUUID((uint16_t)0x2902))
          ->writeValue((uint8_t*)notificationsOn, 2, true);
      Serial.println("Notifications turned on");

      Serial.println("Sending PING1");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(ping1, sizeof(ping1), true);
      delay(200);
      Serial.println("Sending PING2");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(ping2, sizeof(ping2), true);
      delay(200);
      Serial.println("Sending START");
      ConnectedClients[connectedClients]
          ->getService(serviceUUID)
          ->getCharacteristic(writeUUID)
          ->writeValue(start, sizeof(start), true);
      delay(200);
      connectedClients++;
    } else {
      Serial.println("Failed to connect");
    }
  }
}

uint8_t currentLevel = 1;
int counter = 0;
void loop() {
  for (int i = 0; i < connectedClients; i++) {
    if (!ConnectedClients[i]->isConnected()) {
      continue;
    }
#ifdef DEBUG
    Serial.println("Asking for details");
#endif
    ConnectedClients[i]
        ->getService(serviceUUID)
        ->getCharacteristic(writeUUID)
        ->writeValue(details, sizeof(details), true);
    counter++;
  }
  delay(800);  // do a delay to keep things ticking over
}