#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

const IPAddress apIP(192, 168, 2, 1);
const char* apSSID = "TOORX";
boolean settingMode;
String ssidList;

DNSServer dnsServer;
ESP8266WebServer webServer(80);


// Auti einai i metavliti pou stelnoume se kathe request
float currentSpeed;
int st;
int fn;
int var;
int W;
float B;
int analogPin= 0;
int data[3000];



void setup() {
  Serial.begin(115200);
  delay(10);
  settingMode = true;
  setupMode();
}

void loop() {
  
  if (settingMode) {
    dnsServer.processNextRequest();
  }
  
  delay(10);
  webServer.handleClient();
  delay(10);
  
  data[var]= analogRead(analogPin);
  delay(1);
  //Serial.println(data[var]);
  if(data[var]<200 && data[var-1]>700){
   st=var;
  }

   if(data[var]>700 && data[var-1]<200 && st>0){
   fn=var;
  }
  var++;
  Serial.println(var);
  if(var == 3000){
    W=fn-st;
    //Serial.println(W);
    currentSpeed=-0.124*W+28.3;
    //Serial.println(currentSpeed);
    //Serial.println(" ");
    if(W==0){
      currentSpeed=0;
    }

    if(currentSpeed<0){
      currentSpeed=B;
    }

    if(currentSpeed>30){
      currentSpeed=B;
    }
    
    B=currentSpeed;
    var = 1;
    st=0;
    fn=0;
    W=0;
  }
}

void startWebServer() {
  webServer.on("/", handle_OnConnect);
  webServer.onNotFound(handle_NotFound);

  webServer.begin();
  Serial.println("HTTP server started");
  webServer.begin();
}

void setupMode() {
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  Serial.print(apSSID);
  Serial.println("\"");
}

void handle_OnConnect() {
   char currentSpeed_char[10];
   // dtostrf(float var, total string len, decimal points, output)
   dtostrf(currentSpeed, 9, 4, currentSpeed_char);
   Serial.printf("Current speed (char): %s | Current speed (float): %f\n", currentSpeed_char,currentSpeed);
   webServer.send(200, "text/html", currentSpeed_char);
}

void handle_NotFound(){
   webServer.send(404, "text/plain", "Not found");
}
