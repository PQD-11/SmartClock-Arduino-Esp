#include <Arduino.h>
#include <string.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <NTPtimeESP.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <EEPROM.h>
#include <Max72xxPanel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Website.h>

#define ADAFRUIT_USERNAME "username"
#define ADAFRUIT_KEY      "token"

#define WIFI_SSID         "username"
#define WIFI_PASSWORD     "password"

// MATRIX LED defination
#define pinCS                       D4
#define numberOfHorizontalDisplays  4
#define numberOfVerticalDisplays  	1
#define BUFFER_SIZE                 45
char  time_value[BUFFER_SIZE];
char  temp_value[BUFFER_SIZE];
Max72xxPanel  LED_Matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

// Buzzer pin defination
#define BUZZER_PIN  D0

// DHT11 defination
#define DHT_PIN   D1
#define DHT_TYPE  DHT11
DHT Dht(DHT_PIN, DHT_TYPE);
float fCelsius, fHumidity;

// RGB LED WS2812B defination
#define RGB_LED_PIN   D2
int LEDcl[8][3] = {{255, 255, 255}, {255, 0, 0}, {255, 51, 0}, {255, 255, 0}, {0, 153, 0}, {0, 153, 255}, {51, 0, 153}, {102, 0, 153}};
Adafruit_NeoPixel RGB_LED = Adafruit_NeoPixel(8, RGB_LED_PIN, NEO_RGB + NEO_KHZ800);

WiFiClient    espClient;
PubSubClient  Client_Adafruit(espClient);
//PubSubClient  Client_Flespi(espClient);

WiFiUDP wUDP;
NTPClient NTPc(wUDP, "1.asia.pool.ntp.org", 7*3600);
NTPtime NTPt("1.asia.pool.ntp.org");
strDateTime Date;

// Times variables
String  sTime;

// Clock status
bool  isOn = true;
bool  isAlarmSet = false;
int   iClock_Mode = 1;
int   iRGB_LED_Mode = 1;

#define BUTTON_PIN  D3

char  temp_Hours[2], temp_Minutes[2], temp_Second[2];
byte  bDays;

int iLoop = 0, iLoop_Print_Temp_Hum = 0, iLoop_buzzer = 0;

const char  *ssid = "SmartClock";
const char  *password = "SmartClock";
ESP8266WebServer  server(80);

void LED_Matrix_Print(int iCase) {
  String  sTemp_temp = String(fCelsius, 0);
  String  sHum_temp = String(fHumidity, 0);
  String sClear = "      ";
  switch (iCase)
  {
  case 1: // Temperature (Celcius)
    sTemp_temp = "T:" + sTemp_temp;
    sTemp_temp.toCharArray(temp_value, 5);
    LED_Matrix.drawChar(1, 0, temp_value[0], HIGH, LOW, 1); 
    LED_Matrix.drawChar(7, 0, temp_value[1], HIGH, LOW, 1); 
    LED_Matrix.drawChar(14, 0, temp_value[2], HIGH, LOW, 1); 
    LED_Matrix.drawChar(20, 0, temp_value[3], HIGH, LOW, 1);
    LED_Matrix.drawChar(27, 0, temp_value[4], HIGH, LOW, 1);
    LED_Matrix.write();
    break;
  case 2: // Humidity
    sHum_temp = "H:" + sHum_temp;
    sHum_temp.toCharArray(temp_value, 5);
    LED_Matrix.drawChar(1, 0, temp_value[0], HIGH, LOW, 1); 
    LED_Matrix.drawChar(7, 0, temp_value[1], HIGH, LOW, 1); 
    LED_Matrix.drawChar(14, 0, temp_value[2], HIGH, LOW, 1); 
    LED_Matrix.drawChar(20, 0, temp_value[3], HIGH, LOW, 1);
    LED_Matrix.drawChar(27, 0, temp_value[4], HIGH, LOW, 1);
    LED_Matrix.write();
    break;  
  case 3: // Clear
    sClear.toCharArray(temp_value, 6);
    LED_Matrix.drawChar(0, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.drawChar(5, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.drawChar(10, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.drawChar(15, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.drawChar(20, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.drawChar(25, 0, sClear[0], HIGH, LOW, 1);
    LED_Matrix.write();
    break;
  default:  // Times (Hours, Minutes, Second)
    sTime.trim(); // Delete all space in this string
    sTime.toCharArray(time_value, 10);
    LED_Matrix.drawChar(2, 0, time_value[0], HIGH, LOW, 1); // H
    LED_Matrix.drawChar(8, 0, time_value[1], HIGH, LOW, 1); // HH
    LED_Matrix.drawChar(13, 0, time_value[2], HIGH, LOW, 1); // HH:
    LED_Matrix.drawChar(18, 0, time_value[3], HIGH, LOW, 1); // HH:M
    LED_Matrix.drawChar(24, 0, time_value[4], HIGH, LOW, 1); // HH:MM
    LED_Matrix.write();  
    break;
  }

  //LED_Matrix_Print(3);
}

int HEX_TO_INT(char ch) {
  switch (ch) {
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;   
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
        return 10;
    case 'b':
        return 11;
    case 'c':
        return 12;
    case 'd':
        return 13; 
    case 'e':
        return 14;
    case 'f':
        return 15;
    default:
        return 0;
    }
}

void RGB_LED_HEX(int iR, int iG, int iB) {
  RGB_LED.setPixelColor(0, iG, iR, iB);
  RGB_LED.setPixelColor(1, iG, iR, iB);
  RGB_LED.setPixelColor(2, iG, iR, iB);
  RGB_LED.setPixelColor(3, iG, iR, iB);
  RGB_LED.setPixelColor(4, iG, iR, iB);
  RGB_LED.setPixelColor(5, iG, iR, iB);
  RGB_LED.setPixelColor(6, iG, iR, iB);
  RGB_LED.setPixelColor(7, iG, iR, iB);
  RGB_LED.show();
}

void RGB_LED_FROM_SERVER(byte* payload, unsigned int length) {
  // Convert Hex from server Adarfruit to R, G, B value
  byte  temp_R[2];
  temp_R[0] = (char)payload[1];
  temp_R[1] = (char)payload[2];

  byte  temp_G[2];
  temp_G[0] = (char)payload[3];
  temp_G[1] = (char)payload[4];

  byte  temp_B[2];
  temp_B[0] = (char)payload[5];
  temp_B[1] = (char)payload[6];

  int iRed = HEX_TO_INT((char)temp_R[0]) * 16 + HEX_TO_INT((char)temp_R[1]);
  int iGreen = HEX_TO_INT((char)temp_G[0]) * 16 + HEX_TO_INT((char)temp_G[1]);
  int iBlue = HEX_TO_INT((char)temp_B[0]) * 16 + HEX_TO_INT((char)temp_B[1]);

  Serial.println(iRed);
  Serial.println(iGreen);
  Serial.println(iBlue);  
  RGB_LED_HEX(iRed, iGreen, iBlue);
}

void getTimeAlarm(byte* payload) {
  temp_Hours[0] = (char)payload[0];
  temp_Hours[1] = (char)payload[1];

  temp_Minutes[0] = (char)payload[3];
  temp_Minutes[1] = (char)payload[4];

  temp_Second[0] = (char)payload[6];
  temp_Second[1] = (char)payload[7]; 
}

void getDayAlarm(byte* payload, unsigned int length) {
  char  temp_alarm[length];
  for (unsigned int i = 0; i < length; i++) {
    temp_alarm[i] = (char)payload[i];
  }

  String  temp_day = temp_alarm;

  if (temp_day == "today" || temp_day == "Today") {
    bDays = Date.day;
  }
  else if (temp_day == "next day" || temp_day == "Next day" || temp_day == "tomorrow" || temp_day == "Tomorrow") {
    bDays = Date.day + 1;
  }
  else {
    if (temp_day == "monday" || temp_day == "Monday") {
      bDays = 2;
    }
    if (temp_day == "tuesday" || temp_day == "Tuesday") {
      bDays = 3;
    }
    if (temp_day == "wednesday" || temp_day == "Wednesday") {
      bDays = 4;
    }
    if (temp_day == "thursday" || temp_day == "Thursday") {
      bDays = 5;
    }
    if (temp_day == "friday" || temp_day == "Friday") {
      bDays = 6;
    }
    if (temp_day == "saturday" || temp_day == "Saturday") {
      bDays = 7;
    }
    if (temp_day == "sunday" || temp_day == "Sunday") {
      bDays = 1;
    }
  }
}

bool Compare_Alarm() {
  char  temp[8];
  sTime.toCharArray(temp, 8);
  
  if ((temp[0] == temp_Hours[0] && temp[1] == temp_Hours[1]) &&
      (temp[3] == temp_Minutes[0] && temp[4] == temp_Minutes[1]) &&
      Date.day == bDays)
    return true;
  else 
    return false;
}

void Callback_Adafruit(char* topic, byte* payload, unsigned int length) {
  String temp = topic;
  Serial.print("Adafruit: ");
  for (unsigned int i = 0; i < length; i++)
    Serial.print((char)payload[i]);

  Serial.println();

  Serial.println(topic);
  Serial.println(temp);

  if (temp == "minhtan/feeds/ClockStatus") {
    Serial.println("go");
    if (payload[0] == 'o') {
      Serial.println("ON CLOCK");
      isOn = true;
      iClock_Mode = 1;
      RGB_LED.setBrightness(40);
      for (int i = 0; i < 8; i++)
      {
        RGB_LED.setPixelColor(i, LEDcl[i][0], LEDcl[i][1], LEDcl[i][2]);
        RGB_LED.show();
      } 
      RGB_LED.show();

      //Client_Adafruit.publish("minhtan/feeds/RGB_LED_Mode", "1");
      //Client_Adafruit.publish("minhtan/feeds/View", "o");
    }
    else if (payload[0] == 'f') {
      Serial.println("OFF CLOCK");
      isOn = false;
      iClock_Mode = 0;
      RGB_LED.setBrightness(0);
      RGB_LED.show();
      LED_Matrix_Print(3);
    }
  }
  else if (temp == "minhtan/feeds/RGB_LED_Mode") {
    Serial.println("Change LED Mode");
    if (isOn) {
      // RGB LED Normal
      if (payload[0] == '1') {
        iClock_Mode = 1;
        iRGB_LED_Mode = 1;
        RGB_LED.setBrightness(40);

        for (int i = 0; i < 8; i++)
        {
          RGB_LED.setPixelColor(i, LEDcl[i][0], LEDcl[i][1], LEDcl[i][2]);
          RGB_LED.show();
        } 
        //Client_Adafruit.publish("minhtan/feeds/View", "o");
      }
      // RGB LED depends on Temperature
      else if (payload[0] == '2') {
        iClock_Mode = 4;
        iRGB_LED_Mode = 2;
      }
      // Selection from Adafruit Server
      else {
        Serial.println("Hex");
        iClock_Mode = 2;
        iRGB_LED_Mode = 3;
        RGB_LED_FROM_SERVER(payload, length);
      }
    }
  }
  else if (temp == "minhtan/feeds/View") {
    Serial.println("Show temperature");
    if (isOn) {
      if (payload[0] == 't') {
        LED_Matrix_Print(3);
        iClock_Mode = 3;
        iRGB_LED_Mode = 2;
        //Client_Adafruit.publish("minhtan/feeds/RGB_LED_Mode", "2");
      }
      else {
        LED_Matrix_Print(3);
        iClock_Mode = 1;
        iRGB_LED_Mode = 1;

        //Client_Adafruit.publish("minhtan/feeds/RGB_LED_Mode", "1");
      }
    }
  }
  else if (temp == "minhtan/feeds/Alarm_ON_OFF") {
      if ((char)payload[3] == 'n') {
        isAlarmSet = true;
      }
      else if ((char)payload[3] == 'f') {
        isAlarmSet = false;
      }
    }
  else if (temp == "minhtan/feeds/Alarm_Times") {
      getTimeAlarm(payload);
    }
    else if (temp == "minhtan/feeds/Alarm_Days") {
      getDayAlarm(payload, length);
    }
}

void RGB_LED_Config() {
  RGB_LED.begin();
  RGB_LED.setBrightness(40);
  RGB_LED.show();

  for (int i = 0; i < 8; i++)
  {
    RGB_LED.setPixelColor(i, LEDcl[i][0], LEDcl[i][1], LEDcl[i][2]);
    RGB_LED.show();
  } 
}

void MATRIX_LED_Config() {
  LED_Matrix.setIntensity(2);
  LED_Matrix.setRotation(0, 1);
  LED_Matrix.setRotation(1, 1);
  LED_Matrix.setRotation(2, 1);
  LED_Matrix.setRotation(3, 1);
  LED_Matrix.write();
}

void Web_ConnectPage() {
  String s = MAIN_page;
  server.send(200, "text/html", s);
}

void Web_Connected() {
  server.send(200, "text/plain", "Done!");
}

void setupWiFi_Web() {
  String SSID = server.arg("WIFI_SSID");
  String PASS = server.arg("WIFI_PASS");

  if (SSID.length() > 0 && PASS.length() > 0) {
    // Clean EEPROM
    for (int i = 0; i < 96; ++i)
      EEPROM.write(i, 0); 

    // Write data (WIFI_SSID and WIFI_PASS)
    for (unsigned int i = 0; i < SSID.length(); ++i)
      EEPROM.write(i, SSID[i]);
    
    for (unsigned int i = 0; i < PASS.length(); ++i)
      EEPROM.write(32 + i, PASS[i]);

    EEPROM.commit();
  }

  ESP.reset();

  server.send(200, "text/plain", "Connecting succesfully!");
}

bool Connect_WIFI_First() {
  int index = 0;
  while (index < 30) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
  
    Serial.println("Connecting (Firstly)");
    delay(1000);
    index++;
  }
  Serial.println("Timeout!");
  return false;
}

void WiFi_Config() {
  EEPROM.begin(512);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  String  SSID = "";
  String  PASS = "";

  // Read WiFi SSID from EEPROM
  for (int i = 0; i < 32; ++i) {
    SSID += char(EEPROM.read(i));
  }
  Serial.println(SSID);

  // Read WiFi Password form EEPROM
  for (int i = 32; i< 96; i++) {
    PASS += char(EEPROM.read(i));
  }
  Serial.print(PASS);

  WiFi.begin(SSID, PASS);
  if (Connect_WIFI_First()) {
    server.on("/", Web_Connected);
    server.begin();
  }
  else {
    WiFi.softAP(ssid, password);
    server.on("/", Web_ConnectPage);
    server.on("/setupWiFi", setupWiFi_Web);
    server.begin();   
  }

  Serial.println(WiFi.localIP());
  
}

void Adafruit_Server_Config() {
  Client_Adafruit.setServer("io.adafruit.com", 1883);
  Client_Adafruit.setCallback(Callback_Adafruit);
  Client_Adafruit.connect("SmartClock", ADAFRUIT_USERNAME, ADAFRUIT_KEY);
  Client_Adafruit.subscribe("minhtan/feeds/+");  
}

void RGB_LED_Depend_Temperature() {
if(fCelsius >= 20.00 && fCelsius <= 22.00)
    {
      RGB_LED.setPixelColor(0, 0, 0, 255);
      RGB_LED.setPixelColor(1, 0, 0, 235);
      RGB_LED.setPixelColor(2, 0, 0, 215);
      RGB_LED.setPixelColor(3, 10, 0, 190);
      RGB_LED.setPixelColor(4, 0, 0, 0);
      RGB_LED.setPixelColor(5, 0, 0, 0);
      RGB_LED.setPixelColor(6, 0, 0, 0);
      RGB_LED.setPixelColor(7, 0, 0, 0);
    }
    else if (fCelsius > 23.00 && fCelsius <= 25.00)
    {
      RGB_LED.setPixelColor(0, 0, 0, 255);
      RGB_LED.setPixelColor(1, 0, 0, 235);
      RGB_LED.setPixelColor(2, 0, 0, 215);
      RGB_LED.setPixelColor(3, 10, 0, 190);
      RGB_LED.setPixelColor(4, 100, 0, 130);
      RGB_LED.setPixelColor(5, 0, 0, 0);
      RGB_LED.setPixelColor(6, 0, 0, 0);
      RGB_LED.setPixelColor(7, 0, 0, 0);
    }
    else if (fCelsius > 25.00 && fCelsius <= 29.00)
    {
      Serial.println(fCelsius);
      RGB_LED.setPixelColor(0, 0, 0, 255);
      RGB_LED.setPixelColor(1, 0, 0, 235);
      RGB_LED.setPixelColor(2, 0, 0, 215);
      RGB_LED.setPixelColor(3, 50, 0, 190);
      RGB_LED.setPixelColor(4, 100, 0, 130);
      RGB_LED.setPixelColor(5, 175, 0, 50);
      RGB_LED.setPixelColor(6, 0, 0, 0);
      RGB_LED.setPixelColor(7, 0, 0, 0);
    }
    else if (fCelsius > 29.00 && fCelsius <= 32.00)
    {
      RGB_LED.setPixelColor(0, 0, 0, 255);
      RGB_LED.setPixelColor(1, 0, 0, 235);
      RGB_LED.setPixelColor(2, 0, 0, 215);
      RGB_LED.setPixelColor(3, 50, 0, 190);
      RGB_LED.setPixelColor(4, 100, 0, 130);
      RGB_LED.setPixelColor(5, 125, 0, 50);
      RGB_LED.setPixelColor(6, 200, 0, 10);
      RGB_LED.setPixelColor(7, 0, 0, 0);
    }
    else if (fCelsius > 32)
    {
      RGB_LED.setPixelColor(0, 0, 0, 255);
      RGB_LED.setPixelColor(1, 0, 0, 235);
      RGB_LED.setPixelColor(2, 0, 0, 215);
      RGB_LED.setPixelColor(3, 50, 0, 190);
      RGB_LED.setPixelColor(4, 100, 0, 130);
      RGB_LED.setPixelColor(5, 125, 0, 50);
      RGB_LED.setPixelColor(6, 200, 0, 10);
      RGB_LED.setPixelColor(7, 255, 0, 0);
    }
  RGB_LED.show();
}

void Publishing_Data() {
  Serial.println(fCelsius);
  Serial.println(fHumidity);
  char  sTemp_temp[4], sHumidity_temp[4];
  String(fCelsius, 0).toCharArray(sTemp_temp, 4);
  String(fHumidity, 0).toCharArray(sHumidity_temp, 4);

  Client_Adafruit.publish("minhtan/feeds/Temperature", sTemp_temp);
  Client_Adafruit.publish("minhtan/feeds/Humidity", sHumidity_temp);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  RGB_LED_Config();
  MATRIX_LED_Config();

  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("Connecting");
  //   delay(1000);
  // }
  Serial.println(WiFi.localIP());
  server.begin();

  WiFi_Config();
  Adafruit_Server_Config();
  // Client_Adafruit.publish("minhtan/feeds/ClockStatus", "o");
  // Client_Adafruit.publish("minhtan/feeds/RGB_LED_Mode", "1");

  NTPc.begin();
  Dht.begin();
}

void loop() {
  server.handleClient();

  Client_Adafruit.loop();

  NTPc.update();
  sTime = NTPc.getFormattedTime();
  Date  = NTPt.getNTPtime(7.0, 0);

  fCelsius = Dht.readTemperature();
  fHumidity = Dht.readHumidity();
  Serial.println(digitalRead(BUTTON_PIN));

  if (isOn) {
    if (iLoop < 60) {
      iLoop++;
    }
    else {
      Publishing_Data();
      iLoop = 0;

      Serial.println(sTime);
    }
    if (iClock_Mode == 1) {
      LED_Matrix_Print(0);
    }
    else if (iClock_Mode == 2) {
      LED_Matrix_Print(0);
    }
    else if (iClock_Mode == 3) {
      if (iLoop_Print_Temp_Hum >= 0 && iLoop_Print_Temp_Hum < 5) {
        LED_Matrix_Print(1);
        iLoop_Print_Temp_Hum++;
      }
      else if (iLoop_Print_Temp_Hum == 5) {
        Serial.println("clear");
        LED_Matrix_Print(3);
        iLoop_Print_Temp_Hum++;
      }
      else if (iLoop_Print_Temp_Hum > 5 && iLoop_Print_Temp_Hum <= 10)
      {
        Serial.println("Humi");
        LED_Matrix_Print(2);
        iLoop_Print_Temp_Hum++;
      }
      if (iLoop_Print_Temp_Hum == 10) iLoop_Print_Temp_Hum = 0;
      
      RGB_LED_Depend_Temperature();
    }
    else if (iClock_Mode == 4) {
      LED_Matrix_Print(0);
      RGB_LED_Depend_Temperature();
    }
  }

  if (isAlarmSet) {
    if (Compare_Alarm()) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(10000);
      digitalWrite(BUZZER_PIN, LOW);
      isAlarmSet = false;
      Client_Adafruit.publish("minhtan/feeds/Alarm_ON_OFF", "A_Off");     
    }
  }

  delay(1000);
}