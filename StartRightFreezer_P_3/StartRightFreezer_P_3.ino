#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPLuD0TCJ5b"//"TMPLwuOJ53QD"
#define BLYNK_DEVICE_NAME "Freezer"//"NodemcuBlynk"
#define BLYNK_AUTH_TOKEN "MZVFPs1GFvZN4CzHxyrSBX3NK2sX5M-S"//"mvtQmRxPGszWtmIfvYzUsnj-jipnKu7R"
//char auth[] = "mvtQmRxPGszWtmIfvYzUsnj-jipnKu7R";
#define BLYNK_MAX_SENDBYTES 256

//#include <WebSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEsp32.h>
#include <FS.h>
#include <SPIFFS.h>
/* DS18B20 Temperature Sensor */
#include <OneWire.h>
#include<DallasTemperature.h>
#include <WiFiClientSecure.h>
#include "datetime.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <time.h>
#include <ESP32Ping.h>
//include <ESPmDNS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPmDNS.h>
#include <WebSerial.h>

/* TIMER */
#include <SimpleTimer.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */
#define FORMAT_SPIFFS_IF_FAILED true
SimpleTimer timer;
#define ONE_WIRE_BUS 15 // DS18B20 on arduino pin2 corresponds to D4 on physical board
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
int temp;
int notificationOn = 0;
//WiFiManager wm;
#include <ESPAsyncWebServer.h>

bool onBoardLedState = true;
WiFiManager wm;
unsigned long previousMillis = 0;
unsigned long interval = 30000;
unsigned long prevMillis = 0;
unsigned long inter = 30000;

String readString;
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbxWseVkDUe0giGf3wbmVcngtQ5lEWKFNpUQnmLrBtSav4B3Zi04GF63eW42Z0wVnrhahA";  // Replace by your GAS service id

char ssid[] = "RoodRouter";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;
//AKfycbyWlzmqUNeVsapmy9heun8QYF0VLSdUS8SZmzC37teEk85rGXY0HH63NR6B15p-Tfxqag  the one that worked Start_Right_Freezer
void sendToGoogleSheets(String temperature);
int* setCurrentTime();
void getSendData();
//void sendData(float tem, int hum);
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

const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CST6CDT,M3.5.0/02:00:00,M10.5.0/03:00:00";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)

tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

String currentDate;
unsigned long epochTime;
int *curTimeArr;

//const char* ntpServer = "pool.ntp.org";
bool offline = false;
int offlineCount;
int offlineFinalCount;
//bool wifiConnected = false;
int dly = 0;
int dly2 = 0;
unsigned long OFFLINE_SAVE_INTERVAL = 900000;
unsigned long SIMPLE_TIMER = 30000;
bool firstTimeOffline = true; 
unsigned long offlineStart = 0;

int newDay = 0;
bool Connected2Blynk = false;

WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
AsyncWebServer server(80);

/*unsigned long previousMillis = 0;
unsigned long interval = 30000;*/
//unsigned long prevMillis = 0;
unsigned long intv = 30000;
bool wifiConnected = false;



void Wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info){
  //Serial.println("Successfully connected to Access Point");
  //Serial.print
}

void Get_IPAddress(WiFiEvent_t event, WiFiEventInfo_t info){
  //Serial.println("WIFI is connected!");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  WebSerial.println(WiFi.localIP());
  wifiConnected = true;
}

void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  //Serial.println("Disconnected from WIFI access point");
  //Serial.print("WiFi lost connection. Reason: ");
  //Serial.println(info.disconnected.reason);
  //Serial.println("Reconnecting...");
  WiFi.begin();
}
//////////////declare functions//////////////
//void printDateTime(const RtcDateTime& dt);
int getCurrentTime(String input);
String readOfflineTemps(String count);
void writeOfflineTemps(String count, String url);
void startSpiffs();
void writeFile(fs::FS &fs, const char * path, const char * message);
String readFile(fs::FS &fs, const char * path);
bool isDaylightSavingsTime();
void sendData(int tem, int hum, bool fromSpiff,String inputUrl);
void saveParamsCallback ();
void connectWifi();
void setTheTime(tm localTime);
bool getNTPtime(int sec);
bool isDateTimeValid();
void printDateTime(RtcDateTime now);
void setRtcTime();
void saveParamsCallback ();
String getNextOfflineCount();
bool pinged();

////////////////////////////////////////////
 
void setup(){
  Serial.begin(115200);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    // put your setup code here, to run once:
    Serial.begin(115200);
    //reset settings - wipe credentials for testing
    //wm.resetSettings();
   // wm.addParameter(&custom_mqtt_server);
    wm.setConfigPortalBlocking(true);
    //wm.setSaveParamsCallback(saveParamsCallback);
    wm.setConfigPortalTimeout(60);

    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("Start_Right_Freezer")){
        Serial.println("connected...yeey :)");
        //WebSerial.println("connected...yeey :)");
        //wm.process();
    }
    else {
        Serial.println("Configportal running");
    }
      Serial.println("local ip");
    Serial.println(WiFi.localIP());
    //WebSerial.println(WiFi.localIP());
 delay(1000);

  WiFi.onEvent(Wifi_connected,SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(Get_IPAddress, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(Wifi_disconnected, SYSTEM_EVENT_STA_DISCONNECTED); 
  WiFi.begin();
  //Serial.println("Waiting for WIFI network...");

 delay(1000);
 WebSerial.begin(&server);
 server.begin();
 getNTPtime(10);
 setTheTime(timeinfo);
 setRtcTime();

  //////////////////////////////////////////////////////////////////////

////////////////////////////// NTPClient ////////////////////////////
  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);


////////////////////////////////////////////////////////

/////////////////////////////// RTC //////////////////////
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            //Serial.print("RTC communications error = ");
            //Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            //Serial.println("RTC lost confidence in the DateTime!");

            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue

            Rtc.SetDateTime(compiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        //Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        //Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        //Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        //Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
//////////////////////////////////////////////
  DS18B20.begin();

  timer.setInterval(900000L, getSendData);//TODO put back at 900000
  //Blynk.notify("Freezer Temperature");
      Blynk.config(BLYNK_AUTH_TOKEN);


  startSpiffs();

}

void loop(){
  delay(1000);
  timer.run();
  wm.process();

  if(pinged()){
    wifiConnected = true;
    Blynk.run();  //will try reconnecting automatically
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >=interval) {
      Serial.print("N");
      WebSerial.print("N");
      //Serial.println("local ip");
      //Serial.println(WiFi.localIP());

      previousMillis = currentMillis;
    }
  }else{
    wifiConnected = false;
    unsigned long curMillis = millis();
    if(curMillis - prevMillis >=intv) {
      Serial.print("F");
      WebSerial.print("F");
      prevMillis = curMillis;
    }
  }


  if(wifiConnected){  //////////////////////////// ONLINE /////////////////////////////////////
    dly++;
    if(dly >=30){
      Serial.print("N");
      WebSerial.print("N");
      //getNTPtime(10);
      //setTheTime(timeinfo);
      dly = 0;
    }
    if(onBoardLedState){
      onBoardLedState = false;
      digitalWrite(2,0);
    }else{
      onBoardLedState = true;
      digitalWrite(2,1);
    }

  
    //client = server.available();

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
    ////////////////////////////////////////////////////
    //  going to try to write the offline data now we are online
    ////////////////////////////////////////////////////////////////////
     String offlineCountStr = readOfflineTemps("/count");
     if(offlineCountStr != ""){
      int readCount = offlineCountStr.toInt();
     
      for(int i=readCount;i>0;i--){
        String strCnt = String(i);
        String filename = "/"+strCnt;
        String url = readOfflineTemps(filename);
        if(url != ""){
          sendData(0, 0, true, url);
          SPIFFS.remove("/"+i);
          Serial.print("Just save offline data and removed  count = /");
          Serial.print(i);
          String temp = String(i-1);
          Serial.print("Just save the new count: ");
          Serial.print(temp);
          WebSerial.println("Just saved offline stuff now back online");
          writeOfflineTemps("/count",temp.c_str());
        }
        delay(3000);
      } 
     }else{ /////////////////////////// OFFLINE //////////////////////////////////////////////////
      //Serial.println("The offline count was blank THIS COULD BE AN ISSUE");
     }
    /////////////////////////////////////////////////////////////////
    
    delay(500);
   }else{
    //getNTPtime(10);
    if(firstTimeOffline){
      offlineStart = millis();
      firstTimeOffline = false;
    }else if((millis() - offlineStart) >= OFFLINE_SAVE_INTERVAL){

      Serial.print("O");
      WebSerial.print("O");
      //getNTPtime(10);
      //setTheTime(timeinfo);
      setTheTime(timeinfo);
      offlineStart = 0;
      firstTimeOffline = true;
      //TODO write offline stuff
         //Serial.print("Offline save file count is: ");
        //Serial.println(offlineCount);   
    }
   }
  if(newDay != getCurrentTime("day")){
    Serial.println("IT'S A NEW DAY!!!!!!!!!!");
    WebSerial.println("IT'S A NEW DAY!!!!!!!!!!");
    newDay = getCurrentTime("day");
    if(wifiConnected){
      getNTPtime(10);
       setTheTime(timeinfo);
        setRtcTime();
      Serial.println("Get Time");
      Serial.print("Year is: ");
      Serial.println(getCurrentTime("year"));
      Serial.print("Month is: ");
      Serial.println(getCurrentTime("month"));
      Serial.print("Day is: ");
      Serial.println(getCurrentTime("day"));
      Serial.print("Hour is: ");
      Serial.println(getCurrentTime("hour"));
      Serial.print("Minute is: ");
      Serial.println(getCurrentTime("minute"));
      Serial.print("Second is: ");
      Serial.println(getCurrentTime("second"));
  }
  }
  
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
    
    setTheTime(timeinfo);
    sendData(temp,0,false,"");

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

////////////////////////////////////////////////////////////////////////////////
// Subroutine for sending data to Google Sheets
///////////////////////////////////////////////////////////////////////////
void sendData(int tem, int hum, bool fromSpiff,String inputUrl) {
  String url = "";
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google 
  client.setInsecure();
  getNTPtime(10);
  setTheTime(timeinfo);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    offline = true;
    
  }else{
    offline = false;
  }
  //----------------------------------------
 
  //----------------------------------------Processing data and sending data
  if(inputUrl == ""){
    String string_temperature =  String(tem);
    // String string_temperature =  String(tem, DEC); 
    String string_humidity =  String(hum, DEC);
    //?month=2&&day=21&&interval=5&&temperature=22 TODO
    String strDay = String(getCurrentTime("day"));
    String strMonth = String(getCurrentTime("month"));
    int interval = 0;
    if(getCurrentTime("hour") == 24){
      currentHour =0;
    }
    currentHour = getCurrentTime("hour");
    currentMinute = getCurrentTime("minute");
    if(currentMinute < 15){
      interval = currentHour*3+1;
    }else if(currentMinute >=15 && currentMinute <30){
      interval = currentHour*3+2;
    }else if(currentMinute >=30 && currentMinute <45){
      interval = currentHour*3+3;
    }else if(currentMinute >=45 && currentMinute <60){
      interval = currentHour*3+4;
    }
    String strInterval = String(interval);
  //TODO  String strHour = String(currentHour);
    url = "/macros/s/" + GAS_ID + "/exec?month=" +strMonth+"&day="+strDay+"&interval="+strInterval+"&temperature=" + string_temperature;
    Serial.print("built URL: ");
    Serial.println(url);
  }else{
    url = inputUrl;
    Serial.print("saved URL: ");
    Serial.println(url);
    WebSerial.println(url);
  }
  if(wifiConnected){
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
      //write this file to offline memory
      String c = getNextOfflineCount();
      writeOfflineTemps("/count",c);
      String filename = "/"+c;
      writeOfflineTemps(filename,url);

    }
    Serial.println(line);
    Serial.print("reply was : ");
    Serial.println(line);
    Serial.println("closing connection");
    Serial.println("==========");
    Serial.println();
  }else{
    int numericCount = 0;
    String cnt = readOfflineTemps("/count");
    if(cnt != ""){
      numericCount = cnt.toInt();
      numericCount++;
      Serial.print("in sendData offline count is: ");
      Serial.println(numericCount);
      String strCnt = String(numericCount);
      String filename = "/"+strCnt;
      writeOfflineTemps(filename,url);
      String cnt = String(numericCount);
      writeOfflineTemps("/count",cnt);
    }else{
      writeOfflineTemps("/1",url);
      String one = String(1);
      writeOfflineTemps("/count",one);
      Serial.print("in sendData offline count wasn't found in spiffs so count is count is: ");
      Serial.println(1);
    }
  }
}

bool isDaylightSavingsTime() {
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







////////////////////////////////////////////////////////////////////
//
//  Fuction: readFile
//
//  Input:  SPIFFS,
//          char[]  filename path i.e. "/String.txt"
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
String readFile(fs::FS &fs, const char * path) {
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    //Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  //Serial.println(fileContent);
  file.close();
  return fileContent;
}

////////////////////////////////////////////////////////////////////
//
//  Fuction: writeFile
//
//  Input:  SPIFFS,
//          char[] filename path i.e. "/String.txt"
//          String to store
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Writing file: %s\r\n", path);
  //Serial.print("path is : ");
  //Serial.println(path);
  //fs.remove(path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    //WebSerial.println("?EW"); //TODO change to right code
    return;
  }
  
  if (file.print(message)) {
    //Serial.println("- file written");
    if ((strcmp(path, "/timezone.txt") == 0)) {
      //Serial.println("Timezone changed!!!!!!!!11");
    }

  } else {
    Serial.println("- write failed");
  }
}



//////////////////////////////////////////////////////////////
//                                                          //
//   startSpiffs                                            //
//                                                          //
//   input: none                                            //
//                                                          //
//   output: none                                           //
//                                                          //
//   description:  starts of the file disk system.          //
//                                                          //
//////////////////////////////////////////////////////////////
void startSpiffs() {
  int count = 0;
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    while((!SPIFFS.begin()) && (count < 10)){
      Serial.print("SPPPFFF");
      count++;
    }
    return;
  }

}

void writeOfflineTemps(String count, String url){
  //Serial.print("in writeSpiff and count is: ");
  //Serial.println(count);
    //Serial.print("in writeSpiff and url is: ");
  //Serial.println(url);
  writeFile(SPIFFS, count.c_str(), url.c_str());
}

String readOfflineTemps(String count){
  return readFile(SPIFFS,count.c_str());
}

int getCurrentTime(String input){

    
    if(input == "year"){
      int year = timeinfo.tm_year + 1900;
      return year;
    }else if(input == "month"){
      int month = timeinfo.tm_mon + 1;
      return month;
    }else if(input == "day"){
      int day = timeinfo.tm_mday;
      return day;
    }else if(input == "hour"){
      int hour = timeinfo.tm_hour;
      return hour;
    }else if (input == "minute"){
      int minute = timeinfo.tm_min;
      return minute;
    }
}



void connectWifi() {
/*  Blynk.disconnect(); //reset Blynk connection
  delay(100);
  
  Serial.println("Starting Wifi/Blynk");*/

  Blynk.begin(auth, ssid, pass);
  delay(100);

  if (Blynk.connected()) {
    Connected2Blynk = true;
    Serial.println("Wifi/Blynk started");
  }
  else {
    //Serial.println("Check Router");
  }
}





void setTheTime(tm localTime) {
  if(wifiConnected){
    Serial.print("Day is: ");
    currentDay = localTime.tm_mday;
    Serial.println(currentDay);
    Serial.print("Month is: ");
    currentMonth = localTime.tm_mon + 1;
    Serial.println(currentMonth);
    Serial.print("Year is: ");
    currentYear = localTime.tm_year - 100;
    Serial.println(currentYear);
    Serial.print("Hour is: ");
    currentHour = localTime.tm_hour;
    Serial.println(currentHour);
    Serial.print("Minute is: ");
    currentMinute = localTime.tm_min;
    Serial.println(currentMinute);
    Serial.print("Second is: ");
    currentSecond = localTime.tm_sec;
    Serial.println(currentSecond);
    Serial.print(" Day of Week ");
    if (localTime.tm_wday == 0)   Serial.println(7);
    else Serial.println(localTime.tm_wday);
  }else{
    RtcDateTime dt = Rtc.GetDateTime();
    currentMonth = dt.Month();
    Serial.print("RTC month is: ");
    Serial.print(currentMonth);
    currentDay = dt.Day();
    Serial.print("RTC day is: ");
    Serial.print(currentDay);
    currentYear = dt.Year();
    Serial.print("RTC year is: ");
    Serial.print(currentYear);
    currentHour = dt.Hour();
    Serial.print("RTC hour is: ");
    Serial.print(currentHour);
    currentMinute = dt.Minute();
    Serial.print("RTC minute is: ");
    Serial.print(currentMinute);
    currentSecond = dt.Second();
    Serial.print("RTC second is: ");
    Serial.print(currentSecond);
  }
}


/*
 // Shorter way of displaying the time
  void showTime(tm localTime) {
  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d, day %d, %s time\n",
    localTime.tm_year + 1900,
    localTime.tm_mon + 1,
    localTime.tm_mday,
    localTime.tm_hour,
    localTime.tm_min,
    localTime.tm_sec,
    (localTime.tm_wday > 0 ? localTime.tm_wday : 7 ),
    (localTime.tm_isdst == 1 ? "summer" : "standard")
  );
  }
*/

bool isDateTimeValid(){
    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            //Serial.print("RTC communications error = ");
            //Serial.println(Rtc.LastError());
            return false;
        }
        else
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            //Serial.println("RTC lost confidence in the DateTime!");
            return false;
        }
    }
    return true;
}

void printDateTime(RtcDateTime now){
   char datestring[20];
  Serial.println("SHOWING RTC TIME**********");
    snprintf_P(datestring, 
            sizeof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            now.Month(),
            now.Day(),
            now.Year(),
            now.Hour(),
            now.Minute(),
            now.Second() );
    Serial.print(datestring);
}

bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      //Serial.print("*");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
    //Serial.print("now ");  
    //Serial.println(now);
    char time_output[30];
    //strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
    //Serial.println(time_output);
    //Serial.println();
  }
  return true;
}

void setRtcTime(){
  RtcDateTime currentTime = RtcDateTime(currentYear, currentMonth, currentDay, currentHour, currentMinute, 0); //define date and time object
  Rtc.SetDateTime(currentTime); //configure the RTC with object
}

void saveParamsCallback () {
  Serial.println("Get Params:");
  Serial.print(custom_mqtt_server.getID());
  Serial.print(" : ");
  Serial.println(custom_mqtt_server.getValue());
}

String getNextOfflineCount(){
      int numericCount = 0;
    String cnt = readOfflineTemps("/count");
    if(cnt != ""){
      numericCount = cnt.toInt();
      numericCount++;
      Serial.print("in sendData offline count is: ");
      Serial.println(numericCount);
      String strCnt = String(numericCount);
      String cnt = String(numericCount);
      return cnt;
    }else{
      return "1";  //first one
    }
}

bool pinged(){
  bool success = Ping.ping("www.google.com", 3);
  delay(3000);
  if(!success){
   // Serial.println("Ping failed");
    return false;
  }else{
    //Serial.println("Ping succesful.");
    return true;
  }

}
