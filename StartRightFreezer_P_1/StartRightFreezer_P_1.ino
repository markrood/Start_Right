/*************************************************************

*************************************************************/

/* Comment this out to disable prints and save space */
//curl -X POST -H "Content-Type: application/json" -d {\"value1\":\"999\"} https://maker.ifttt.com/trigger/Freezer/with/key/fBplW8jJqqotTqTxck4oTdK_oHTJKAawKfja-WlcgW-



#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPLuD0TCJ5b"//"TMPLwuOJ53QD"
#define BLYNK_DEVICE_NAME "Freezer"//"NodemcuBlynk"
#define BLYNK_AUTH_TOKEN "MZVFPs1GFvZN4CzHxyrSBX3NK2sX5M-S"//"mvtQmRxPGszWtmIfvYzUsnj-jipnKu7R"
//char auth[] = "mvtQmRxPGszWtmIfvYzUsnj-jipnKu7R";
#define BLYNK_MAX_SENDBYTES 256
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager//#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>

/* DS18B20 Temperature Sensor */
#include <OneWire.h>
#include<DallasTemperature.h>
#include <WiFiClientSecure.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
/* TIMER */
#include <SimpleTimer.h>


SimpleTimer timer;
#define ONE_WIRE_BUS 4 // DS18B20 on arduino pin2 corresponds to D4 on physical board
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
int temp;
int notificationOn = 0;
WiFiServer server(80);

bool onBoardLedState = true;

String readString;
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbxWseVkDUe0giGf3wbmVcngtQ5lEWKFNpUQnmLrBtSav4B3Zi04GF63eW42Z0wVnrhahA";  // Replace by your GAS service id

//AKfycbyWlzmqUNeVsapmy9heun8QYF0VLSdUS8SZmzC37teEk85rGXY0HH63NR6B15p-Tfxqag  the one that worked Start_Right_Freezer
void sendToGoogleSheets(String temperature);
int* setCurrentTime();
void getSendData();
void sendData(float tem, int hum);
float moData[120];
long notificationTimer = 3600000;
long beginNotificationTimer = 0;
bool firstTimeNotification = true;
    long utcOffsetInSeconds = 0; 

//int randomTemp = 0;
bool sent = false;
float tab2 = 0;
int x = 0;

    int monthDay;
    int currentMonth;
    String currentMonthName;
    int currentYear;
    String formattedTime;
    int currentDay;
    int currentHour;
    int currentMinute;
    int currentSecond;
    String dayWeek;

// You should get Auth Token in the Blynk App.
//// Go to the Project Settings (nut icon).
//char auth[] = BLYNK_AUTH_TOKEN;

//String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
//String payload = "";

//WiFiClient client = server.available();
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

String currentDate;
unsigned long epochTime;
int *curTimeArr;
struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  digitalWrite(2,1);
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();

  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Start_Right_Sliders");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  // Setup Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  while (Blynk.connect() == false) {
  }
  DS18B20.begin();

  timer.setInterval(900000L, getSendData);
  //Blynk.notify("Freezer Temperature");

  timeClient.begin();
  int daylightOffset_sec = 0;
  if(isDaylightSavingsTime()){
    timeClient.setTimeOffset(-18000);
  }else{
    timeClient.setTimeOffset(-21600);
  }

}

void loop()
{
  if(onBoardLedState){
    onBoardLedState = false;
    digitalWrite(2,0);
  }else{
    onBoardLedState = true;
    digitalWrite(2,1);
  }
  //randomTemp = random(-10, 32);
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("LOST WiFi restarting");
    //atoAwcUtil->webSerialWriteLine("LOST WiFi restarting");
    ESP.restart();
  }
  timeClient.update();
  
  //client = server.available();
  timer.run(); // Initiates SimpleTimer
  Blynk.run();
  String _temp = String(temp);  //TODO prod  back to temp

  if ((temp >= tab2) && notificationOn) { //TODO prod back to temp

    if (firstTimeNotification) {
      firstTimeNotification = false;
      beginNotificationTimer = millis();
      Blynk.logEvent("freezer_temperature", "Start Rite your freezer is above minus 10 degrees it is " + _temp);
    }
    if ((millis() - beginNotificationTimer) >= notificationTimer) {
      Blynk.logEvent("freezer_temperature", "Start Rite your freezer is above minus 10 degrees it is " + _temp);
      //https://blynk.cloud/external/api/logEvent?token=mvtQmRxPGszWtmIfvYzUsnj-jipnKu7R&code=freezer_temperature
      firstTimeNotification = true;
    }
  }
  // Serial.print(".");
  delay(6000);
}

/***************************************************
   Send Sensor data to Blynk
 **************************************************/
void getSendData()
{

  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  temp = (temp * 9.0) / 5.0 + 32;
  String t = String(temp);
  moData[x] = temp;
  Serial.println(temp);
  //  Serial.println(t);
  Blynk.virtualWrite(10, temp); //virtual pin V10
  x++;
  Serial.println(x);

    // Blynk.email("markdamonrood@gmail.com","april temp data",unpackData());
    Serial.print("going to send trigger with ");
    Serial.println(t);
    
    updateTime();
    sendData(temp,0);

}

BLYNK_WRITE(V2) {
  tab2 = param.asInt();
  Serial.println(tab2);
}

BLYNK_WRITE(V0)  // button attached to Virtual Pin 1 in SWITCH mode
{
  int i = param.asInt();
  if (i == 1) {
    notificationOn = 1;
    Serial.println("Button is ON");
  }
  else {
    notificationOn = 0;
    Serial.println("Button is OFF");
  }
}


// Subroutine for sending data to Google Sheets
void sendData(float tem, int hum) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google 
  client.setInsecure();
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------
 
  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC);
  //?month=2&&day=21&&interval=5&&temperature=22 TODO
  String strDay = String(currentDay);
  String strMonth = String(currentMonth);
  int interval = 0;
  if(currentHour == 0){
    currentHour =24;
  }
  if(currentMinute < 15){
    interval = currentHour*4+1;
  }else if(currentMinute >=15 && currentMinute <30){
    interval = currentHour*4+2;
  }else if(currentMinute >=30 && currentMinute <45){
    interval = currentHour*4+3;
  }else if(currentMinute >=45 && currentMinute <60){
    interval = currentHour*4+4;
  }
  String strInterval = String(interval);
//TODO  String strHour = String(currentHour);
  String url = "/macros/s/" + GAS_ID + "/exec?month=" +strMonth+"&day="+strDay+"&interval="+strInterval+"&temperature=" + string_temperature;
  Serial.print("requesting URL: ");
  Serial.println(url);
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");
 
  Serial.println("request sent");
  //----------------------------------------
 
  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println(line);
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}

bool isDaylightSavingsTime() {
  updateTime();
  int yr = currentYear;
  //Serial.print(yr);
  int dayOfMonth = currentMonth;
  //Serial.print(dayOfMonth);
  int hr = currentHour;
  //Serial.print(hr);
  int mo = currentMonth;
  //Serial.print(mo);
  int DST = 0;

  // ********************* Calculate offset for Sunday *********************
  int y = yr;                          // DS3231 uses two digit year (required here)
  int x = (y + y / 4 + 2) % 7;    // remainder will identify which day of month
  // is Sunday by subtracting x from the one
  // or two week window.  First two weeks for March
  // and first week for November
  // *********** Test DST: BEGINS on 2nd Sunday of March @ 2:00 AM *********
  if (mo == 3 && dayOfMonth == (14 - x) && hr >= 2)
  {
    DST = 1;                           // Daylight Savings Time is TRUE (add one hour)
  }
  if ((mo == 3 && dayOfMonth > (14 - x)) || mo > 3)
  {
    DST = 1;
  }
  // ************* Test DST: ENDS on 1st Sunday of Nov @ 2:00 AM ************
  if (mo == 11 && dayOfMonth == (7 - x) && hr >= 2)
  {
    DST = 0;                            // daylight savings time is FALSE (Standard time)
  }
  if ((mo == 11 && dayOfMonth > (7 - x)) || mo > 11 || mo < 3)
  {
    DST = 0;
  }
  if (DST == 1)                       // Test DST and add one hour if = 1 (TRUE)
  {
    hr = hr + 1;
  }
  if (DST == 1) {
    return true;
  } else {
    return false;
  }
}





/*int* setCurrentTime(){
  updateTime();
  curTimeArr[0] = currentYear;
  curTimeArr[1] = currentMonth;
  curTimeArr[2] = currentDay;
  curTimeArr[3] = currentHour;
  curTimeArr[4] = currentMinute;
  return curTimeArr;
}*/

  void updateTime(){
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime(); // was unsigned long
    struct tm *ptm = gmtime ((time_t *)&epochTime);

    currentYear = ptm->tm_year+1900;
    currentMonth = ptm->tm_mon+1;
    currentMonthName = months[currentMonth-1];
    dayWeek = weekDays[timeClient.getDay()];
    currentDay = ptm->tm_mday;
    currentHour = timeClient.getHours();
    currentMinute = timeClient.getMinutes();
    currentSecond = timeClient.getSeconds();
    Serial.print("Current Year is: ");
    Serial.println(currentYear);
    Serial.print("Current Month is: ");
    Serial.println(currentMonth);
    Serial.print("Current Hour is: ");
    Serial.println(currentHour);
    Serial.print("Current Day is: ");
    Serial.println(currentDay);
    Serial.print("Current Minute is: ");
    Serial.println(currentMinute);
/*    formattedTime = _timeClient->getFormattedTime();
    currentHour = _timeClient->getHours();
    currentMinute = _timeClient->getMinutes();
    currentSecond = _timeClient->getSeconds();
    weekDay = weekDays[_timeClient->getDay()];
    currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);*/
  }
