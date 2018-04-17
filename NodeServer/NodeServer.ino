#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* localssid = "ESP1";

// Set web server port number to 80
ESP8266WebServer server(80);

// Variable to store the HTTP request
String header;

String hostname = "ESPtwo";

// Assign output variables to GPIO pins
const int status1 = 14;
const int status2 = 12;
const int output = 4;
const int output2 = 5;

bool status[2] = {false, false};

void root(){
  server.send(200, "text/html", "<h1>Welcome!</h1>");
}

void sendValid() {
  server.send(200, "text/html", "success");
}

void setCreds() {
  String ss = "error";
  String pass = "wrong";
  String thename = "badname";
  resetCreds();
  for(int i = 0; i < ss.length(); ++i){
    EEPROM.write(i, ss[i]);
  }

  for(int i = 0; i < pass.length(); ++i){
    EEPROM.write(32+i, pass[i]);
  }

  for(int i = 0; i < thename.length(); ++i){
    EEPROM.write(64+i, thename[i]);
  }
  EEPROM.commit();
}

void setupMode() {
  // Set outputs to LOW
  digitalWrite(output, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(status1, HIGH);
  digitalWrite(status2, LOW);

  server.stop();
  server.close();

  Serial.println("Starting access point ");
  WiFi.softAP(localssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP Address: ");
  Serial.println(myIP);
  server.on("/", root);
  server.on("/valid",sendValid);
  server.on("/set",[](){
    String id = server.arg("ssid");
    String pass = server.arg("pass");
    String thename = server.arg("name");
    if(id.length() > 0 && pass.length() > 0){
       for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
       for (int i = 0; i < id.length(); ++i){
         EEPROM.write(i, id[i]);
       }
      
       for(int i = 0; i < pass.length(); ++i){
         EEPROM.write(32+i, pass[i]);
       }

       for(int i = 0; i < thename.length(); ++i){
         EEPROM.write(64+i, thename[i]);
       }
       EEPROM.commit();
    }
    server.send(200, "application/json", "{\"success\":\"success\"}");
    WiFi.softAPdisconnect(true);
    startServer();
  });
  server.begin();
}

void resetCreds() {
  for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
  EEPROM.commit();
  setupMode();
}

void start() {
    server.on("/",sendValid);
    server.on("/1/on",[]() {
      status[0] = true;
      digitalWrite(output, LOW);
      server.send(200, "application/json", "{\"success\":\"success\"}");
    });
    server.on("/1/off",[]() {
      status[0] = false;
      digitalWrite(output, HIGH);
      server.send(200, "application/json", "{\"success\":\"success\"}");
    });
    server.on("/2/on",[]() {
      status[1] = true;
      digitalWrite(output2, LOW);
      server.send(200, "application/json", "{\"success\":\"success\"}");
    });
    server.on("/2/off",[]() {
      status[1] = false;
      digitalWrite(output2, HIGH);
      server.send(200, "application/json", "{\"success\":\"success\"}");
    });
    server.on("/status", []() {
      server.send(200, "application/json", "{ \"status1\": "+ String(status[0]) + ", \"status2\": " + String(status[1]) + " }");
    });
    server.on("/reset", []() {
      resetCreds();
      server.send(200, "application/json", "{\"success\":\"success\"}");
    });
}

void startServer() {
  server.stop();
  server.close();

  String savedssid = "";
  String savedpass = "";
  String savedname = "";

  for(int i = 0; i < 32; ++i)
  {
    savedssid += char(EEPROM.read(i));
  }
  Serial.println(savedssid);
  if(savedssid.length() > 1 && savedssid[0] != 0){
    for (int i = 32; i < 64; ++i){
      savedpass += char(EEPROM.read(i));
    }
  }
  Serial.println(savedname);
  for(int i = 64; i < 92; ++i){
    savedname += char(EEPROM.read(i));
  }

  Serial.print("Connecting to ");
  Serial.println(savedssid);
  
  WiFi.hostname(savedname);
  WiFi.begin(savedssid.c_str(), savedpass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(status1, LOW);
  digitalWrite(status2, HIGH);
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  start();
  server.begin();
}

void setup() {
  bool ap = true;
  String savedssid = "";
  String savedpass = "";
  String savedname = "";
  Serial.begin(115200);
  EEPROM.begin(512);
  //setCreds();

  for(int i = 0; i < 32; ++i)
  {
    savedssid += char(EEPROM.read(i));
  }
  Serial.println(savedssid);
  if(savedssid.length() > 1 && savedssid[0] != 0){
    ap = false;
    for (int i = 32; i < 64; ++i){
      savedpass += char(EEPROM.read(i));
    }
    for(int i = 64; i < 92; ++i){
      savedname += char(EEPROM.read(i));
    }
  }
  Serial.println(savedpass);
  Serial.println(savedname);
  // Initialize the output variables as outputs
  pinMode(output, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(status1, OUTPUT);
  pinMode(status2, OUTPUT);
  
  // Set outputs to LOW
  digitalWrite(output, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(status1, HIGH);
  digitalWrite(status2, LOW);

  if(!ap){
    // Connect to Wi-Fi network with SSID and password
    int count = 0;
    Serial.print("Connecting to ");
    Serial.println(savedssid);
    
    WiFi.hostname(savedname);
    WiFi.begin(savedssid.c_str(), savedpass.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      count++;
      if(count > 40) {
        ap = true;
        goto startAP;
      }
    }
    digitalWrite(status1, LOW);
    digitalWrite(status2, HIGH);
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    start();
  }
  startAP:
  if(ap) {
    Serial.println("Starting access point ");
    WiFi.softAP(localssid);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP Address: ");
    Serial.println(myIP);
    server.on("/", root);
    server.on("/valid",sendValid);
    server.on("/set",[](){
      String id = server.arg("ssid");
      String pass = server.arg("pass");
      String thename = server.arg("name");
      if(id.length() > 0 && pass.length() > 0){
         for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
         for (int i = 0; i < id.length(); ++i){
           EEPROM.write(i, id[i]);
         }
        
         for(int i = 0; i < pass.length(); ++i){
           EEPROM.write(32+i, pass[i]);
         }

         for(int i = 0; i < thename.length(); ++i){
           EEPROM.write(64+i, thename[i]);
         }
         EEPROM.commit();
      }
      server.send(200, "application/json", "{\"success\":\"success\"}");
      WiFi.softAPdisconnect(true);
      startServer();
    });
  }
  server.begin();
}

void loop(){
  server.handleClient();
}
