// On ESP8266:
// At 80MHz runs up 57600ps, and at 160MHz CPU frequency up to 115200bps with only negligible errors.
// Connect pin 12 to 14.

#define shutdownpin 13
//#include <HttpClient.h>

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>  
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

#define INFLUXDB_HOST "188.226.154.151"
#define INFLUXDB_USER "brenchies"
#define INFLUXDB_PASS "pastechi"
#define INFLUXDB_DB "pastechi"

#define INFLUXDB_ROW "Particles"
#define INFLUXDB_DEVICE "alpha"

#define WIFI_SSID "Brenchies_Lab"
#define WIFI_PASS "Rietveld0"

#define SLEEPTIME 30e6

//dweet setup
#define DWEET_NAME "BRENCHIESTEST"
#define DWEET_HOST "caribbeanwebsolutions.com"
const char* host = "dweet.io"; //host for tcp connection



ESP8266WiFiMulti WiFiMulti;
Influxdb influx(INFLUXDB_HOST);

 
SoftwareSerial swSer(14, 12, false, 128);

typedef enum {
    FIRSTBYTE,
    SECONDBYTE,
    READ,
    
}dataparcer;

int unitid=2;

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

// Dweet Data Setup
String arrayVariableNames[]={"pm01","pm2_5","pm10","temperature","humidity"}; //THIS WAY WE REFER TO EACH VARIABLE AS AN INDEX IN THIS GLOBAL ARRAY.
int arrayVariableValues[]={0,0,0,0,0}; //initial values
int numberVariables=sizeof(arrayVariableValues)/sizeof(arrayVariableValues[0]); //tells the number of arguments inside each array


void setup() {
  Serial.begin(115200);   //Initialize hardware serial with baudrate of 115200
  swSer.begin(9600);    //Initialize software serial with baudrate of 115200
  Serial.setTimeout(2000);
 
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
  Serial.println("Couldn't find SHT31");
  }
  Serial.println("\nSoftware serial test started");

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WIFI");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);

  Serial.println("Setup done");
  
  pinMode(shutdownpin, OUTPUT);           // set pin to input
  digitalWrite(shutdownpin, HIGH);
  delay(40000);
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
 
    InfluxData row(INFLUXDB_ROW);
    row.addTag("device", INFLUXDB_DEVICE);
    row.addTag("sensor", "one");
    row.addTag("mode", "pwm");
    row.addValue("pm01", PM01Value);
    row.addValue("pm2_5", PM2_5Value);
    row.addValue("pm10", PM10Value);
    row.addValue("temperature", t);
    row.addValue("humidity", h);

    influx.write(row);
    
    dataready = false;

    //Update variable array for dweeting
    arrayVariableValues[0]=PM01Value; //this is how you change values of the variables 
    arrayVariableValues[1]=PM2_5Value; 
    arrayVariableValues[2]=PM10Value; 
    arrayVariableValues[3]=t; 
    arrayVariableValues[4]=h;
    
    dweetdata();//upload it
  
    //ESP.deepSleep(SLEEPTIME);
    
  }
 
}

//connects TCP,sends dweet,drops connection, prints the server response on the console
void dweetdata(){
  // Use WiFiClient class to create TCP connections 
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(DWEET_HOST, httpPort)) { 
    Serial.println("connection failed"); 
    return; 
  } 
  client.print(GetDweetStringHttpBuilder()); 
  //slow doown ...stability stuff
  delay(10);
  // Read all the lines of the reply from dweet server and print them to Serial 
  while(client.available()){ 
    String line = client.readStringUntil('\r'); 
    //Serial.print(line); 
  } 
  //just display ending conection on the serial port 
  Serial.println(); 
  Serial.println("closing connection"); 
}

String GetDweetStringHttpBuilder() {
  int i=0; 
  String dweetHttpGet="GET /brenchies/thedata.php?";//initial empty get request 
  //dweetHttpGet=dweetHttpGet+String(DWEET_NAME)+"?";//start concatenating the thing name (dweet.io) 
  for(i=0;i<(numberVariables);i++){//concatenate all the variable names and values 
    if(i==numberVariables-1){ 
      dweetHttpGet=dweetHttpGet + String(arrayVariableNames[i]) + "="+ String( arrayVariableValues[i]);//the lastone doesnt have a "&" at the end 
    }
    else 
      dweetHttpGet=dweetHttpGet + String(arrayVariableNames[i]) + "="+ String( arrayVariableValues[i]) + "&";   
  }
  dweetHttpGet=dweetHttpGet + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n"; 
  return dweetHttpGet; //this is our freshly made http string request 
}
