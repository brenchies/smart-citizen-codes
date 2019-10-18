// On ESP8266:
// At 80MHz runs up 57600ps, and at 160MHz CPU frequency up to 115200bps with only negligible errors.
// Connect pin 12 to 14.

#define shutdownpin 13

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

#define INFLUXDB_HOST "188.226.154.151"
#define INFLUXDB_USER "brenchies"
#define INFLUXDB_PASS "pastechi"
#define WIFI_SSID "hs-01"
#define WIFI_PASS "4EmployeesOnly100%"

ESP8266WiFiMulti WiFiMulti;
Influxdb influx(INFLUXDB_HOST);

 
SoftwareSerial swSer(14, 12, false, 128);

typedef enum {
    FIRSTBYTE,
    SECONDBYTE,
    READ,
    
}dataparcer;

int transmitPM01(char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[2]<<8) + thebuf[3]); //count PM1.0 value of the air detector module
  return PM01Val;
}
 
//transmit PM Value to PC
int transmitPM2_5(char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[4]<<8) + thebuf[5]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }
 
//transmit PM Value to PC
int transmitPM10(char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[6]<<8) + thebuf[7]); //count PM10 value of the air detector module  
  return PM10Val;
}


void setup() {
  Serial.begin(115200);   //Initialize hardware serial with baudrate of 115200
  swSer.begin(9600);    //Initialize software serial with baudrate of 115200
  Serial.setTimeout(2000);
 
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
  Serial.println("Couldn't find SHT31");
  }

  pinMode(shutdownpin, OUTPUT);           // set pin to input
  digitalWrite(shutdownpin, HIGH);
  delay(30000);
}

char a;
char databuffer[32];
uint8_t i_data=0;
char mystring[10];
bool dataready = false;
dataparcer datastate = FIRSTBYTE;
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module


void loop() {

  while (swSer.available() > 0) {  //wait for data at software serial
    a = swSer.read();
    Serial.print(a);

    switch(datastate){

      case FIRSTBYTE:
        if (a == 0x42) {
          datastate = SECONDBYTE;
         
        }
        break;
      case SECONDBYTE:
        if (a == 0x4d) {
          datastate = READ;
          
        }
        break;
      case READ:
        databuffer[i_data] = a;
        i_data++;
        if(i_data>29){
          datastate = FIRSTBYTE;
          dataready = true;
          i_data = 0;
        }
        break;

      default:
        break;
        
      
    }

    
  }

  if(dataready){
    
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();
    

    
    digitalWrite(shutdownpin, LOW);
    Serial.println("");
    Serial.println("data is:");
    for(uint8_t i=0; i<30;i++){
      Serial.print(databuffer[i], HEX);
      Serial.print(",");
    }
    Serial.println("");
    PM01Value = transmitPM01(databuffer); //count PM1.0 value of the air detector module
    PM2_5Value = transmitPM2_5(databuffer);//count PM2.5 value of the air detector module
    PM10Value = transmitPM10(databuffer); //count PM10 value of the air detector module 

    String pm01 = String(PM01Value);
    String pm2_5 = String(PM2_5Value);
    String pm10 = String(PM10Value);

    Serial.println("PPM01:" + pm01);
    Serial.println("PPM2_5:" + pm2_5);
    Serial.println("PPM10:" + pm10);
    if (! isnan(t)) { // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(t);
    } else {
    Serial.println("Failed to read temperature");
    }
    
    if (! isnan(h)) { // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    } else {
    Serial.println("Failed to read humidity");
    }

    WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WIFI");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDbAuth("pastechi", INFLUXDB_USER, INFLUXDB_PASS);

  Serial.println("Setup done");

    InfluxData row("Particles");
    row.addTag("device", "alpha");
    row.addTag("sensor", "one");
    row.addTag("mode", "pwm");
    row.addValue("pm01", PM01Value);
    row.addValue("pm2_5", PM2_5Value);
    row.addValue("pm10", PM10Value);
    row.addValue("temperature", t);
    row.addValue("humidity", h);
    

    influx.write(row);
    
    dataready = false;

    ESP.deepSleep(30e6);
    
  }
 
}
