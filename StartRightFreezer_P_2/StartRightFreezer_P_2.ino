/*************************************************************

*************************************************************/

/* Comment this out to disable prints and save space */
//curl -X POST -H "Content-Type: application/json" -d {\"value1\":\"999\"} https://maker.ifttt.com/trigger/Freezer/with/key/fBplW8jJqqotTqTxck4oTdK_oHTJKAawKfja-WlcgW-

//https://docs.google.com/spreadsheets/d/1IpNwchi1LXSLpRxJFijwgh_3fwconi6q0eFhn-zmfdo/edit?usp=sharing

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
//#include <WiFiManager.h>
/* DS18B20 Temperature Sensor */
#include <OneWire.h>
#include<DallasTemperature.h>
#include <WiFiClientSecure.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
/* TIMER */
#include <SimpleTimer.h>
#include "FS.h"
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

SimpleTimer timer;
#define ONE_WIRE_BUS 2 // DS18B20 on arduino pin2 corresponds to D4 on physical board
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

WiFiManager wifiManager;
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
bool offline = false;
int offlineCount;
int offlineFinalCount;
bool wifiConnected = false;
int dly = 0;
int dly2 = 0;
unsigned long OFFLINE_SAVE_INTERVAL = 900000;
unsigned long SIMPLE_TIMER = 30000;
bool firstTimeOffline = true; 
unsigned long offlineStart = 0;
bool isDaylightSavingsTime();
int newDay = 0;

 
void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  digitalWrite(2,1);
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around

  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  wifiManager.setConfigPortalBlocking(false);
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //wifiManager.setConfigPortalBlocking(false);
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

  DS18B20.begin();

  timer.setInterval(900000L, getSendData);
  //Blynk.notify("Freezer Temperature");

  timeClient.begin();
  startSpiffs();
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
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");

            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue

            Rtc.SetDateTime(compiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 


}

void loop()
{
    timer.run(); // Initiates SimpleTimer
  if (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("LOST WiFi");
    wifiConnected = false;
  }else{
    if(wifiConnected = false){
      // Setup Blynk
      Blynk.config(BLYNK_AUTH_TOKEN);
      while (Blynk.connect() == false) {
        Serial.print(".");
      }
      ////////////////////////////////rtc////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////

      int daylightOffset_sec = 0;
      if(isDaylightSavingsTime()){
        timeClient.setTimeOffset(-18000);
      }else{
        timeClient.setTimeOffset(-21600);
      }
    }
    wifiConnected = true;
  }

  if(wifiConnected){  //////////////////////////// ONLINE /////////////////////////////////////

    dly++;
    if(dly >=120){
      Serial.print("N");
      dly = 0;
    }
    if(onBoardLedState){
      onBoardLedState = false;
      digitalWrite(2,0);
    }else{
      onBoardLedState = true;
      digitalWrite(2,1);
    }
    timeClient.update();
  
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
          writeOfflineTemps("/count",temp.c_str());
        }
        delay(3000);
      } 
     }else{ /////////////////////////// OFFLINE //////////////////////////////////////////////////
      Serial.println("The offline count was blank THIS COULD BE AN ISSUE");
     }
    /////////////////////////////////////////////////////////////////
    delay(500);
   }else{
    if(firstTimeOffline){
      offlineStart = millis();
      firstTimeOffline = false;
    }else if((millis() - offlineStart) >= OFFLINE_SAVE_INTERVAL){

      Serial.print("O");
      offlineStart = 0;
      firstTimeOffline = true;

         //Serial.print("Offline save file count is: ");
        //Serial.println(offlineCount);   
    }
   }
  wifiManager.process();
  if(newDay != currentDay){
    int daylightOffset_sec = 0;
    if(isDaylightSavingsTime()){
      timeClient.setTimeOffset(-18000);
    }else{
      timeClient.setTimeOffset(-21600);
    }
      //Get a time structure
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int yr = ptm->tm_year+1900;
    int mo = ptm->tm_mon+1; 
    int da= ptm->tm_mday;
    int hr = timeClient.getHours();
    int m = timeClient.getMinutes();
    int s = timeClient.getSeconds();
    RtcDateTime currentTime = RtcDateTime(yr, mo, da, hr, m, s); //define date and time object
    Rtc.SetDateTime(currentTime);
    //RtcDateTime compiled = RtcDateTime();
    newDay = currentDay;
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
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

}

void writeOfflineTemps(String count, String url){
  Serial.print("in writeSpiff and count is: ");
  Serial.println(count);
    Serial.print("in writeSpiff and url is: ");
  Serial.println(url);
  writeFile(SPIFFS, count.c_str(), url.c_str());
}

String readOfflineTemps(String count){
  return readFile(SPIFFS,count.c_str());
}

int getCurrentTime(String input){

    //get rtc
    RtcDateTime now = Rtc.GetDateTime();
    if(input == "Year"){
      return now.Year();
    }else if(input == "month"){
      return now.Month();
    }else if(input == "day"){
      return now.Day();
    }else if(input == "hour"){
      return now.Hour();
    }else if (input == "minute"){
      return now.Minute();
    }

}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            sizeof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
