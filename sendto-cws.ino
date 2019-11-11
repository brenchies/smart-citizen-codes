#include "ESP8266WiFi.h"

// WiFi parameters 
const char* ssid = "ssid"; 
const char* password = "pass"; 
String thingName="thedata.php?unitid=2&"; //thing for grouping all the data together 
//const char* host = "dweet.io"; //host :) for tcp connection
const char* host = "caribbeanwebsolutions.com"; //host :) for tcp connection

// Data stufff
String arrayVariableNames[]={"unitid","pm1","pm2_5","pm10","fahren","humidity","temp"}; //THIS WAY WE REFER TO EACH VARIABLE AS AN INDEX IN THIS GLOBAL ARRAY.
int unitid=2;
int hu=78;
int pm1=2;
int pm2_5=4;
int pm10=4;
int fahren=89;
int arrayVariableValues[]={unitid,pm1,pm2_5,pm10,fahren,hu,99.9};
int numberVariables=sizeof(arrayVariableValues)/sizeof(arrayVariableValues[0]); //tells the number of arguments inside each array

void setup() { 
  Serial.begin(9600);                                    
  
  // We start by connecting to a WiFi network 
  Serial.println(); 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  
  //initialice and connect to wifi lan 
  WiFi.begin(ssid, password);   
  int retries = 0; 
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) { 
    retries++; 
    delay(500); 
    Serial.print("."); 
  } 
  if(retries>14){ 
    Serial.println(F("WiFi conection FAILED")); 
  } 
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.println(F("WiFi connected")); 
    Serial.println("IP address: "); 
    Serial.println(WiFi.localIP()); 
    Serial.println(F("======================================================"));  
  } 
  Serial.println(F("Setup ready"));  
} 

void loop() {  
  arrayVariableValues[0]=1;//this is how you change values of the variables 
  //arrayVariableValues[1]=42;
  //arrayVariableNames[0]="veinticinco";//this is how you change the name of the variable( veinticinco=25) 
  dweetdata();//upload it 
}

//connects TCP,sends dweet,drops connection, prints the server response on the console
void dweetdata(){
  // Use WiFiClient class to create TCP connections 
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) { 
    Serial.println("connection failed"); 
    return; 
  } 
  client.print(GetDweetStringHttpBuilder()); 
  //slow doown ...stability stuff
  delay(1000);
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
  String dweetHttpGet="GET /brenchies/";//initial empty get request 
  dweetHttpGet=dweetHttpGet+String(thingName)+"?";//start concatenating the thing name (dweet.io) 
  for(i=0;i<(numberVariables);i++){//concatenate all the variable names and values 
  if(i==numberVariables-1){ 
    dweetHttpGet=dweetHttpGet +  String(arrayVariableNames[i]) + "="+ String( arrayVariableValues[i]);//the lastone doesnt have a "&" at the end 
  }
  else 
    dweetHttpGet=dweetHttpGet + String(arrayVariableNames[i]) + "="+ String( arrayVariableValues[i]) + "&";   
  }
  dweetHttpGet=dweetHttpGet + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n"; 
  return dweetHttpGet; //this is our freshly made http string request 
}
