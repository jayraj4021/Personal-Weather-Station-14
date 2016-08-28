#include <SoftwareSerial.h>
#include <Wire.h>
#include <dht.h>
#include <Adafruit_BMP085.h>

#define DEBUG 0                                     // change value to 1 to enable debuging using serial monitor  
#define dht_pin A0                                  // defining pin A0 for DHT sensor
#define lightSensor A1                              // defining pin A1 as input pin for LDR voltage divider

dht DHT;
Adafruit_BMP085 bmp;
SoftwareSerial esp8266Module(10, 11);               // RX, TX

String network = "----";                            // your access point SSID
String password = "-------------";                  // your wifi Access Point password
#define IP "184.106.153.149"                        // IP address of thingspeak.com
String GET = "GET /update?key=----------------";    // replace with your channel key


void setup()
{
  if(DEBUG){
    Serial.begin(9600);                             // Setting hardware serial baud rate to 9600
  }  
  esp8266Module.begin(9600);                        // Setting softserial baud rate to 9600
  if (!bmp.begin()) {
    if(DEBUG){
      Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    }
    while (1) {}
  }
  delay(2000);
}

void loop() 
{
    setupEsp8266();                                   
    DHT.read11(dht_pin);
    double humi = DHT.humidity;
    double bmp_temp = bmp.readTemperature();
    double bmp_pressure = bmp.readPressure();
    int lightIntensity = analogRead(lightSensor);
    updateTemp(String(bmp_temp) ,String(lightIntensity),String(bmp_pressure),String(humi));
    delay(30000);
}

//-------------------------------------------------------------------
// Following function setup the esp8266, put it in station made and 
// connect to wifi access point.
//------------------------------------------------------------------
void setupEsp8266()                                   
{
    if(DEBUG){
      Serial.println("Reseting esp8266");
    }
    esp8266Module.flush();
    esp8266Module.println(F("AT+RST"));
    delay(7000);
    if (esp8266Module.find("OK"))
    {
      if(DEBUG){
        Serial.println("Found OK");
        Serial.println("Changing espmode");
      }  
      esp8266Module.flush();
      changingMode();
      delay(5000);
      esp8266Module.flush();
      connectToWiFi();
    }
    else
    {
      if(DEBUG){
        Serial.println("OK not found");
      }
    }
}

//-------------------------------------------------------------------
// Following function sets esp8266 to station mode
//-------------------------------------------------------------------
bool changingMode()
{
    esp8266Module.println(F("AT+CWMODE=1"));
    if (esp8266Module.find("OK"))
    {
      if(DEBUG){
        Serial.println("Mode changed");
      }  
      return true;
    }
    else if(esp8266Module.find("NO CHANGE")){
      if(DEBUG){
        Serial.println("Already in mode 1");
      }  
      return true;
    }
    else
    {
      if(DEBUG){
        Serial.println("Error while changing mode");
      }  
      return false;
    }
}

//-------------------------------------------------------------------
// Following function connects esp8266 to wifi access point
//-------------------------------------------------------------------
bool connectToWiFi()
{
  if(DEBUG){
    Serial.println("inside connectToWiFi");
  }  
  String cmd = F("AT+CWJAP=\"");
  cmd += network;
  cmd += F("\",\"");
  cmd += password;
  cmd += F("\"");
  esp8266Module.println(cmd);
  delay(15000);
  
  if (esp8266Module.find("OK"))
  {
    if(DEBUG){
      Serial.println("Connected to Access Point");
    }  
    return true;
  }
  else
  {
    if(DEBUG){
      Serial.println("Could not connect to Access Point");
    }  
    return false;
  }
}

//-------------------------------------------------------------------
// Following function sends sensor data to thingspeak.com
//-------------------------------------------------------------------
void updateTemp(String voltage1,String voltage2,String voltage3,String voltage4)
{  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  esp8266Module.println(cmd);
  delay(5000);
  if(esp8266Module.find("Error")){
    if(DEBUG){
      Serial.println("ERROR while SENDING");
    }  
    return;
  }
  cmd = GET + "&field1=" + voltage1 + "&field2=" + voltage2 + "&field3=" + voltage3 + "&field4=" + voltage4 + "\r\n";
  esp8266Module.print("AT+CIPSEND=");
  esp8266Module.println(cmd.length());
  delay(15000);
  if(esp8266Module.find(">"))
  {
    esp8266Module.print(cmd);
    if(DEBUG){
      Serial.println("Data sent");
    }
  }else
  {
    esp8266Module.println("AT+CIPCLOSE");
    if(DEBUG){
      Serial.println("Connection closed");
    }  
  }
}
