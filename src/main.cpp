/* This code is to be used on a ESP NodeMCU board. It pulls data in a json format from
the Florida GIS Dashboard pertaining Highlands county.     */

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
      
LiquidCrystal_I2C lcd(0x27,20,4);

const char* ssid = "SSID";         //Name of your WiFi network
const char* password = "PASSWORD"; // WiFi Password

const char* host = "services1.arcgis.com";
String request = "/CY1LXxl9zlJeBuRZ/arcgis/rest/services/Florida_COVID19_Cases/FeatureServer/0/query?where=COUNTYNAME%20%3D%20%27HIGHLANDS%27&outFields=CasesAll,Deaths,NewPos&returnGeometry=false&outSR=4326&f=json";
const int httpsPort = 443;


void setup() {
  
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  Serial.print("Loading...");
  lcd.print("Covid Counter by JC");
  
  Serial.begin(115200);
  Serial.print("Connecting");
  lcd.setCursor(0,1);
  lcd.print("Connecting");

  delay(1000);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue
  delay(1000);
  WiFi.mode(WIFI_STA);        //Station mode
  WiFi.begin(ssid, password);     //Connect to WiFi
  
  while (WiFi.status() != WL_CONNECTED) {  // Wait for connection
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  
  lcd.clear();
  Serial.println("");  //If connection successful show IP address of ESP8266 in serial monitor and LCD
  Serial.print("Connected to: ");
  lcd.print("Connected To: ");
  Serial.println(ssid);
  lcd.print(ssid);
  Serial.print("IP address: ");
  lcd.setCursor(0,1);
  lcd.print("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0,2);
  lcd.print(WiFi.localIP());
}

void loop() {
   
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  Serial.println(host);
  
  //httpsClient.setCACert();
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);

  Serial.println("HTTPS Connecting");
  lcd.clear();
  lcd.print("HTTPS Connecting");

  int r = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (r < 24)) {
    delay(100);
    Serial.print(".");
    lcd.print(".");

    r++;
  }
  
  if (r == 24) {
    Serial.println("Connection failed");
    lcd.clear();
    lcd.print("Connection failed");
  }
  else {
    Serial.println("Connected");
    lcd.clear();
    lcd.print("Fetching Data");
  }

  Serial.print("Requesting: ");
  Serial.println(host + request);

  httpsClient.print(String("GET ") + request + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  Serial.println("Request sent");

  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  Serial.println("Payload received:");

  String payload;
  while (httpsClient.available()) {
    payload = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(payload); //Print response
  }

  Serial.println("Closing connection");

  char charBuf[500];
  payload.toCharArray(charBuf, 500);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 4*JSON_OBJECT_SIZE(7) + 950;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, payload);
        
  JsonObject features_0_attributes = doc["features"][0]["attributes"];
  int features_0_attributes_CasesAll = features_0_attributes["CasesAll"]; // All Cases
  int features_0_attributes_Deaths = features_0_attributes["Deaths"]; // Deaths
  int features_0_attributes_NewPos = features_0_attributes["NewPos"]; // New Positive
    
  Serial.print("Confirmed: ");
  Serial.println(features_0_attributes_CasesAll);
  Serial.print("New Pos: ");
  Serial.println(features_0_attributes_NewPos);
  Serial.print("Deaths: ");
  Serial.println(features_0_attributes_Deaths);
  
  lcd.clear();
  lcd.print("Cases:");
  lcd.setCursor(7, 0);
  lcd.print(features_0_attributes_CasesAll);
  lcd.setCursor(0, 1);
  lcd.print("New:");
  lcd.setCursor(5, 1);
  lcd.println(features_0_attributes_NewPos);
  lcd.setCursor(0, 2);
  lcd.print("Deaths:");
  lcd.setCursor(8, 2);
  lcd.print(features_0_attributes_Deaths);

  delay(3600000);
}
