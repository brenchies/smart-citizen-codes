#include <ESP8266WiFi.h>

// WiFi parameters 
const char* ssid = "Brenchies_Lab"; 
const char* password = "Rietveld0"; 
String thingName="BRENCHIESTEST"; //thing for grouping all the data together 
const char* host = "dweet.io"; //host :) for tcp connection

// Data stufff
String arrayVariableNames[]={"humidity","temperature"}; //THIS WAY WE REFER TO EACH VARIABLE AS AN INDEX IN THIS GLOBAL ARRAY.
int arrayVariableValues[]={99,100};
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
  //arrayVariableValues[0]=25;//this is how you change values of the variables 
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
  String dweetHttpGet="GET /dweet/for/";//initial empty get request 
  dweetHttpGet=dweetHttpGet+String(thingName)+"?";//start concatenating the thing name (dweet.io) 
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
