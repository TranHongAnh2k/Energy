#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPtimeESP.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <time.h>
#include "EEPROM.h"

#include <PZEM004Tv30.h>


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/************************* Oled spi *********************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int measurePin = A0;
int ledPower = 2;
 
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
 
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

/************************* WiFi Access Point *********************************/
String WLAN_SSID;                        //string variable to store ssid
String WLAN_PASS; 
unsigned long rst_millis;
/************************* cloudmqtt Setup *********************************/

#define serveruri "driver.cloudmqtt.com"
#define port       18643
#define username  "cqbfckol"
#define password  "mpSkyZ4D1N6f"

/*************************pin Setup *********************************/

#define LENGTH(x) (strlen(x) + 1)   // length of char string
#define EEPROM_SIZE 200
/*************************PZEM Setup ************************************/
PZEM004Tv30 pzem(12, 14);//RX,TX
float voltage;
float current;
float power;
float energy;
float frequency;
unsigned long readTime;
unsigned long dayTime;
unsigned long timeMqtt;
unsigned long feedBackTime;
unsigned long alarmTime;
unsigned long confirmTime;

/*************************Instance Setup ************************************/
//tạo 1 client
WiFiClient myClient;
//**************************** Server NTP ***********************************
NTPtime NTPch("vn.pool.ntp.org");

strDateTime dateTime;
byte nowHour = 0;     // Gio
byte nowMinute = 0;   // Phut
byte onHour = 0;     // Gio On
byte onMinute = 0;   // Phut On
byte offHour = 0;     // Gio Off
byte offMinute = 0;   // Phut Off

/*************************** Sketch Code ************************************/
void sensorRead();
void reconnect();
/*************************** Smartconfig ************************************/
Ticker ticker;

bool in_smartconfig = false;
void enter_smartconfig()
{
  if (in_smartconfig == false) {
    in_smartconfig = true;
//    ticker.attach(0.1, tick);
    WiFi.beginSmartConfig();
  }
}

void exit_smart()
{
  in_smartconfig = false;
}
/******************************************************************/
//*****************khởi tạo pubsubclient***************************
PubSubClient mqtt(serveruri, port, callback, myClient);

void setup()
{
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  Serial.println("Khoi Tao");
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Do an!");
  display.setCursor(0, 20);
  // Display static text
  display.println("He thong thu thap dien nang");
  display.setCursor(0, 30);
  // Display static text
  display.println("Tran Hong Anh");
  display.display(); 
  WiFi.disconnect();
  EEPROM.begin(EEPROM_SIZE);
  WLAN_SSID = readStringFromFlash(0); // Read SSID stored at address 0
  Serial.print("SSID = ");
  Serial.println(WLAN_SSID);
  WLAN_PASS = readStringFromFlash(40); // Read Password stored at address 40
  Serial.print("psss = ");
  Serial.println(WLAN_PASS);
  WiFi.begin(WLAN_SSID.c_str(), WLAN_PASS.c_str());
  

  Serial.println();
  Serial.println();
  display.setCursor(0, 10);
  // Display static text
  display.clearDisplay();
  display.setCursor(0, 10);
  // Display static text
  display.println("Dang ket noi wifi");
  display.display();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  delay(5000);
  while (WiFi.status() != WL_CONNECTED)
  {
    display.clearDisplay();
   display.setCursor(0, 10);
  // Display static text
   display.println("Che do smart config");
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
     
    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
      display.setCursor(0, 20);
  // Display static text
  display.println("Cho ket noi");
  display.display();
    }

    Serial.println("");
    Serial.println("SmartConfig received.");

    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi Connected.");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // read the connected WiFi SSID and password
    WLAN_SSID = WiFi.SSID();
    WLAN_PASS = WiFi.psk();
    Serial.print("SSID:");
    Serial.println(WLAN_SSID);
    Serial.print("PSS:");
    Serial.println(WLAN_PASS);
    Serial.println("Store SSID & PSS in Flash");
    writeStringToFlash(WLAN_SSID.c_str(), 0); // storing ssid at address 0
    writeStringToFlash(WLAN_PASS.c_str(), 40); // storing pss at address 40
    WiFi.begin(WLAN_SSID.c_str(), WLAN_PASS.c_str());
    delay(5000);
  }
     display.clearDisplay();
     display.setCursor(0, 10);
  // Display static text
    display.println("Ket noi Wifi thanh cong");
     display.display();
     delay(1000);
  // Get MacAddress and remove ":"
  String MacAddress = WiFi.macAddress();
  MacAddress.remove(2,1);
  MacAddress.remove(4,1);
  MacAddress.remove(6,1);
  MacAddress.remove(8,1);
  MacAddress.remove(10,1);
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(MacAddress);
   while(!time(nullptr)){
    Serial.print("#");
    delay(500);
    }
    Serial.println("Time respone is ok!: "); 
  // kết nối với mqtt server
  while (1)
  {
    delay(500);
    if (mqtt.connect("ESP8266", username, password))
      break;
  }
  Serial.println("connected to MQTT server.....");
  
}
void loop()
{
  if (WiFi.status() == WL_DISCONNECTED )
  {
    Serial.println("Restarting the ESP");
    ESP.restart(); 
  }

  //làm mqtt luôn sống
  mqtt.loop();

  //phản hồi trạng thái relay lên server
  if (mqtt.connected())
  {
    if (millis()>dayTime +60000){
        timeCheck();
    }
    //check if 5 seconds has elapsed since the last time we read the sensors.
    if (millis() > readTime + 5000)
    {
      sensorRead();
      
    }
  }

}
//************************* hàm kiem tra thời gian *****************
void timeCheck(){
   dayTime = millis(); 
   time_t now = time(nullptr);
   struct tm* p_tm=localtime(&now);
   int ngay=p_tm ->tm_mday;
   Serial.print("Ngay");
   Serial.println(ngay);
   if (ngay == 1){
      display.clearDisplay();
      display.setCursor(0, 10);
      display.print("Reset dien ap tieu thu");
      display.display();
      pzem.resetEnergy();
      delay(5000); 
   }
 }
//************************* hàm đọc giá trị sensor *****************
void sensorRead()
{
  readTime = millis();
  voltage = pzem.voltage();//V
  current = pzem.current();//A
  power = pzem.power();//W
  energy = pzem.energy();//kW.h
  frequency = pzem.frequency();//Hz
  display.clearDisplay();
  Serial.print("Dien ap: ");
  Serial.println(voltage);
   display.setCursor(0, 10);
  // Display static text
  display.print("Dien ap:");
  display.print(voltage);
  display.println("V");
   display.setCursor(0, 20);
  // Display static text
  display.print("Dong dien:");
  display.print(current);
  display.println("A");
  display.setCursor(0, 30);
  // Display static text
  display.print("Cong suat:");
  display.print(power);
  display.println("kW");
  display.setCursor(0, 40);
  // Display static text
  display.print("Cong suat tieu thu:");
  display.print(energy);
  display.println("kWh");
  display.display();
  if (millis()>timeMqtt+60000){
    timeMqtt= millis();
  // Get MacAddress and remove ":"
  String MacAddress = WiFi.macAddress();
  MacAddress.remove(2,1);
  MacAddress.remove(4,1);
  MacAddress.remove(6,1);
  MacAddress.remove(8,1);
  MacAddress.remove(10,1);
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(MacAddress);

  // init mqttChannel
  String mqttMainChannel = "ESPs/enviroment/";
  String mqttChannel =MacAddress;

  // convert data to JSON
  StaticJsonDocument<200> doc;
  JsonObject content  = doc.to<JsonObject>();
  doc["voltage"] = voltage;
  doc["current"]   = current;
  doc["energy"]   = energy;
 
  char content_string[256];
  serializeJson(content, content_string);

  // push data to mqtt
  mqtt.publish(mqttChannel.c_str(), content_string);
  }

}

//********************* hàm trả dữ liệu về *******************************
void callback(char *tp, byte *message, unsigned int length)
{
  
}

//*********************** hàm reconnect **********************
void reconnect()
{
  // lặp đến khi kết nối lại
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting connection...");
    WiFi.reconnect();
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("reconnected");
      return;
    }
    else
    {
      Serial.print("failed to connect WiFi!!");
      Serial.println(" try again in 5 seconds...");
      // chờ 5s
      delay(5000);
    }
  }

  while (!mqtt.connected())
  {
    Serial.println("Attempting connection...");
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (mqtt.connected())
    {
      Serial.println("reconnected");
        String MacAddress = WiFi.macAddress();
        MacAddress.remove(2,1);
        MacAddress.remove(4,1);
        MacAddress.remove(6,1);
        MacAddress.remove(8,1);
        MacAddress.remove(10,1);
        Serial.print("ESP8266 Board MAC Address:  ");
        Serial.println(MacAddress);
      return;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // chờ 5s
      delay(5000);
    }
  }
}



void writeStringToFlash(const char* toStore, int startAddr) {
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}


String readStringFromFlash(int startAddr) {
  char in[128]; // char array of size 128 for reading the stored data 
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}
