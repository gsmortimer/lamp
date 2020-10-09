/*********
  George Mortimer
*********/

/* Descrition
 *  
 *  Lamp controller
 *  clicking the button toggles output on/off, and updates server to be on or off.
 *  Queries server to see if lamp is on or off.
 *  If at first server is not same as output, server is ignored
 *  If server matches output, server will start being responded to
 *  If server status changes, lamp will change.
 *  setup() is run on boot to establish Wifi connection, and report status to UART
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>




// Network Details
const char* ssid1 = "WIFI1";
const char* password1 = "supersecret";
const char* ssid2 = "WIFI2";
const char* password2 = "supersecret";
const char* ssid3 = "WIFI3";
const char* password3 = "supersecret";


// Server to connect to
const char* host = "www.example.com";
const uint16_t port = 80;
const char* statuspage =  "GET /lamp/lamp.txt HTTP/1.1\r\nhost: www.example.com\r\nConnection: close\r\n\r\n";
const char* setstatuson =  "GET /lamp/lamp.php?lamp=on HTTP/1.1\r\nhost: www.example.com\r\nConnection: close\r\n\r\n";
const char* setstatusoff = "GET /lamp/lamp.php?lamp=off HTTP/1.1\r\nhost: www.example.com\r\nConnection: close\r\n\r\n";


// Start webserver on localhost
//ESP8266WebServer server(80);
String webPage = "";
String response = "";
bool switched = false;
bool relaystate = true;
int waitCount = 20;

int gpio13Led = 13;
int gpio12Relay = 12;
int gpio0switch = 0;
uint8_t counter = 0;

//Setup client for sending out data on port
WiFiClient client;


      
ESP8266WiFiMulti wifiMulti;

//boot time setup
void setup(void){  
  // preparing GPIOs
  WiFi.persistent(false);
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);  
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, HIGH);
  pinMode(gpio0switch, INPUT);
 
  Serial.begin(115200); 
  delay(2000);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid1, password1);
  wifiMulti.addAP(ssid2, password2);
  wifiMulti.addAP(ssid3, password3);
  wifiMulti.run(); //Start Wifi
  Serial.println("");
  // Wait for connection
 // while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
 //   Serial.print(WiFi.status());
   // WiFi.printDiag(Serial);
 // }
//  while (WiFi.config(staticIP, gateway, subnet, gateway) != true) { 
 //   Serial.print("Failed to set IP");
//  }
  delay(500);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



// Create connection to web server if not already connected
int connection(void) {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
 
  // create TCP connection to webserver 
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    waitCount = 200; //reduce number of requests if server connection failed.
    delay(100);
    return 1;
  }
  Serial.println("connected");
  waitCount = 20; //normal number of requests if server connection succeeded.
  return 0;
}

// Send string of data to server
int senddata(String text) {
  // This will send a string to the server
  Serial.println("Checking connection...");
  if (!client.connected()) {
    connection();
  }
  if (client.connected()) {
    Serial.println("Sending Data...");
    client.print(text);
    return 0;
  }
  return 1;
}

//Get Full Server Response
void getdata () {
  response = "";
  client.setTimeout (400);
  while (client.connected() || client.available())
  {
    if (client.available())
    {
      response += client.readStringUntil('\n');
      response += "<br>";
    }
   Serial.print(":"); 
  }
}

//Get Just Page (Single Line after whitespace line) Todo: more than just line
String getpage () {
  response = "none";
  client.setTimeout (400);
  while (client.connected() || client.available())
  {
    if (client.available())
    {
      if (response == "\r") return client.readStringUntil('\0');
      else response = client.readStringUntil('\n');
    }
    Serial.print(".");
  }
}
 
void loop(void){
  //server.handleClient();
  
  if (counter > waitCount) { // every 20 (2secs) or 200 (20secs)  cycles check server
    senddata(statuspage);
    response=getpage();
     
    if (response=="on\n" && switched == 1) { //check server 
      relaystate=true;   
    }
    if (response=="off\n" && switched == 1) { //check server 
      relaystate=false;
    }
    counter = 1;
    // reconnect to wifi if dropped
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("WiFi not connected!");
    }
  }
  counter++;
  
  if ((response == "on\n" && relaystate == true) || (response == "off\n" && relaystate == false)) {
    switched = 1; //If server matches device, device will respond to server.
  }
  
  if (digitalRead(gpio0switch)==0) { // switch pressed
     if (relaystate==false) { 
      relaystate = true;
      digitalWrite(gpio13Led, LOW);
      digitalWrite(gpio12Relay, HIGH);
      if (senddata(setstatuson) == 1){ //update server but ignore server temporarily if it failed
        Serial.println("Failed to update server");
        switched = 0;
      }

     } else if (relaystate==true) { 
      relaystate = false;
      digitalWrite(gpio13Led, HIGH);
      digitalWrite(gpio12Relay, LOW);
      if (senddata(setstatusoff) == 1){ //update server but ignore server temporarily if it failed
        Serial.println("Failed to update server");
        switched = 0;
      }
     }
     while (digitalRead(gpio0switch)==0) {
      delay (100); //debounce
     }
     counter = 1;
     waitCount = 20; //normal number of requests if button pressed
     
  }
 
  if (relaystate==true) {   //update relay and led states
    digitalWrite(gpio13Led, LOW);
    digitalWrite(gpio12Relay, HIGH);
  }
  if (relaystate==false) {
    digitalWrite(gpio13Led, HIGH);
    digitalWrite(gpio12Relay, LOW);
  }
  delay(100);
} 
