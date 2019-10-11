#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_MPL3115A2.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

#include "index.h"


//SSID and Password of your WiFi router
const char* ssid = "hs-01";
const char* password = "4EmployeesOnly100%";
#define bh1730 0x29 


ESP8266WebServer server(80); //Server on port 80


float getbaro(void){
  baro.begin();
  
  float pascals = baro.getPressure();
  // Our weather page presents pressure in Inches (Hg)
  // Use http://www.onlineconversion.com/pressure.htm for other units
  Serial.print(pascals/3377); Serial.println(" Inches (Hg)");

  float altm = baro.getAltitude();
  Serial.print(altm); Serial.println(" meters");
  return pascals;
}

float getlight(void){

  uint8_t TIME0  = 0xDA;
      uint8_t GAIN0 = 0x00;
      uint8_t DATA [8] = {0x03, TIME0, 0x00 ,0x00, 0x00, 0xFF, 0xFF ,GAIN0} ;
      
      uint16_t DATA0 = 0;
      uint16_t DATA1 = 0;
      
      Wire.beginTransmission(bh1730);
      Wire.write(0x80|0x00);
      for(int i= 0; i<8; i++) Wire.write(DATA[i]);
      Wire.endTransmission();
      delay(100); 
      Wire.beginTransmission(bh1730);
      Wire.write(0x94);  
      Wire.endTransmission();
      Wire.requestFrom(bh1730, 4);
      DATA0 = Wire.read();
      DATA0=DATA0|(Wire.read()<<8);
      DATA1 = Wire.read();
      DATA1=DATA1|(Wire.read()<<8);
        
      uint8_t Gain = 0x00; 
      if (GAIN0 == 0x00) Gain = 1;
      else if (GAIN0 == 0x01) Gain = 2;
      else if (GAIN0 == 0x02) Gain = 64;
      else if (GAIN0 == 0x03) Gain = 128;
      
      float ITIME =  (256- TIME0)*2.7;
      
      float Lx = 0;
      float cons = (Gain * 100) / ITIME;
      float comp = (float)DATA1/DATA0;

      
      if (comp<0.26) Lx = ( 1.290*DATA0 - 2.733*DATA1 ) / cons;
      else if (comp < 0.55) Lx = ( 0.795*DATA0 - 0.859*DATA1 ) / cons;
      else if (comp < 1.09) Lx = ( 0.510*DATA0 - 0.345*DATA1 ) / cons;
      else if (comp < 2.13) Lx = ( 0.276*DATA0 - 0.130*DATA1 ) / cons;
      else Lx=0;

      return Lx;
}


void handleRoot() {
  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  float l = getlight();
  float p = getbaro();
  
  String websiteopen = "<HTML><HEAD><TITLE>My first web page</TITLE></HEAD><BODY><CENTER>Sensor Display</CENTER>";
  String websitemiddle = "<p>Temperature is:"+String(t)+"</p><p>Humidity is:"+String(h)+"</p><p>Light is:"+String(l)+"</p><p>pressure is:"+String(p)+"</p>";
  String websiteclose = "</BODY></HTML>";
  String s;
  s = websiteopen+websitemiddle+websiteclose;
  
  server.send(200, "text/html", s);
}

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

while (!Serial)
delay(10); // will pause Zero, Leonardo, etc until serial console opens

Serial.println("SHT31 test");
if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
Serial.println("Couldn't find SHT31");
while (1) delay(1);
}

WiFi.begin(ssid, password);     //Connect to your WiFi router
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
Serial.println("connected <3");
Serial.println(WiFi.localIP());



server.on("/", handleRoot);
   
server.begin();  


}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient(); 
}
