#include <Arduino.h>
#include "user_interface.h"// for os_timer
#include "FS.h"// for open file on flash


#include <WebSocketsServer.h>// Socket server library in 'lib' folder
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define TIMER_UPDATE 5000

os_timer_t myTimer;
const char* ssid = "Livebox-75C0";
const char* password = "ipW2j3EzJQg6LF9Er6";


#define D0 16
#define D1 5
#define D2 4
#define D3 0 //Forbiden
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15



void start_sequence()
{
  digitalWrite(D7, LOW);
  digitalWrite(D5, HIGH);
  delay(1000);
  digitalWrite(D5, LOW);
  digitalWrite(D6, HIGH);
  delay(1000);
  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  delay(1000);
  digitalWrite(D7, LOW);

}

void setup() {
  // WiFi.softAP(ssid, password);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, INPUT_PULLUP);
  // put your setup code here, to run once:
}

void loop() {
  if (!digitalRead(D8))
  {
    start_sequence();
  }
  // put your main code here, to run repeatedly:
}