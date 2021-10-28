#include <Arduino.h>
#include "user_interface.h"// for os_timer
#include "FS.h"// for open file on flash


#include <WebSocketsServer.h>// Socket server library in 'lib' folder
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define TIMER_UPDATE 5000

os_timer_t myTimer;
os_timer_t startLightTimer;
const char* ssid = "RC_Startline";
const char* password = "88888888";

int8_t currentStartSequence = -1;

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

#define D0 16
#define D1 5
#define D2 4
#define D3 0 //Forbiden
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

int16_t connected_socket_clients[256] = {-1};

void fixedLoop(void *pArg);

// void start_sequence()
// {


// }

void add_knowed_socket_client(uint8_t num)
{
	for (int16_t i = 0; i < 256; i++)
	{
		if (connected_socket_clients[i] == num)
		{
			break;
		}
		if (connected_socket_clients[i] == -1)
		{
			connected_socket_clients[i] = num;
			break;
		}
	}
}

void ICACHE_RAM_ATTR startSequence(void)
{
	Serial.println("In isr");;
	if (currentStartSequence == 3)
	{
		digitalWrite(D7, LOW);
		digitalWrite(D5, HIGH);
		currentStartSequence--;
	}
	else if (currentStartSequence == 2)
	{
		digitalWrite(D5, LOW);
		digitalWrite(D6, HIGH);
		currentStartSequence--;
	}
	else if (currentStartSequence == 1)
	{
		digitalWrite(D6, LOW);
		digitalWrite(D7, HIGH);
		currentStartSequence--;
	}
	else if (currentStartSequence == 0)
	{
		timer1_disable();
		currentStartSequence--;
	}
	else 
	{
		timer1_disable();
	}
}


void engageStart()
{
	if (currentStartSequence == -1)
	{
		currentStartSequence = 3;
		timer1_attachInterrupt(startSequence);
		//Clock = 80 Mhz (80 000 000 Hz)
		timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
		//1 000 000 = 0.2s
		timer1_write(5000000); // 1.0s



		// os_timer_setfn(&startLightTimer, startSequence, NULL);
		// os_timer_arm(&startLightTimer, 1000, true);
	}
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
	//Serial.println("socket received");
	if (type == WStype_TEXT)
	{
		// for(size_t i = 0; i < length; i++)
		// {
		// 	Serial.print((char) payload[i]);
		// }
		
		Serial.println((char*)payload);
		Serial.println(num);
		add_knowed_socket_client(num);
		if (strncmp((char*)payload, "[START]", sizeof("[START]") - 1) == 0)
		{
			engageStart();
		}
		else if (strncmp((char*)payload, "[STOP]", sizeof("[STOP]") - 1) == 0)
		{
			digitalWrite(D5, HIGH);
			digitalWrite(D6, LOW);
			digitalWrite(D7, LOW);
		}
		else if (strncmp((char*)payload, "[OFF]", sizeof("[OFF]") - 1) == 0)
		{
			digitalWrite(D5, LOW);
			digitalWrite(D6, LOW);
			digitalWrite(D7, LOW);
		}
		else if (strncmp((char*)payload, "[RED]", sizeof("[RED]") - 1) == 0)
		{
			if (strstr((char*)payload, "ON"))
			{
				digitalWrite(D5, HIGH);
			}
			else if (strstr((char*)payload, "OFF"))
			{
				digitalWrite(D5, LOW);
			}
		}
		else if (strncmp((char*)payload, "[ORANGE]", sizeof("[ORANGE]") - 1) == 0)
		{
			if (strstr((char*)payload, "ON"))
			{
				digitalWrite(D6, HIGH);
			}
			else if (strstr((char*)payload, "OFF"))
			{
				digitalWrite(D6, LOW);
			}
		}
		else if (strncmp((char*)payload, "[GREEN]", sizeof("[GREEN]") - 1) == 0)
		{
			if (strstr((char*)payload, "ON"))
			{
				digitalWrite(D7, HIGH);
			}
			else if (strstr((char*)payload, "OFF"))
			{
				digitalWrite(D7, LOW);
			}
		}
		// Serial.println();
	}
}

void setup() {
	// WiFi.softAP(ssid, password);
	Serial.begin(115200);

 	for (int16_t i = 0; i < 256; i++)
	{
		connected_socket_clients[i] = -1;
	}


	WiFi.softAP(ssid, password);
	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	SPIFFS.begin();
	if (!MDNS.begin("startline"))
	{
		Serial.println("dns error");
	}
	else
	{
		Serial.println("dns ok");
	}
	server.serveStatic("/", SPIFFS, "/index.html");
	server.serveStatic("/main.js", SPIFFS, "/main.js");
	server.serveStatic("/index.css", SPIFFS, "/index.css");
	server.begin();
	MDNS.addService("http", "tcp", 80);

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);

	pinMode(D5, OUTPUT);
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
	pinMode(D8, INPUT_PULLUP);

	os_timer_setfn(&myTimer, fixedLoop, NULL);
	os_timer_arm(&myTimer, TIMER_UPDATE, true);
	
	// put your setup code here, to run once:
}

void fixedLoop(void *pArg)// delay function didnt work in this scope
{
	// webSocket.sendTXT("test from slave");
	for (int16_t i = 0; i < 256; i++)
	{
		if (connected_socket_clients[i] != -1)
		{
			Serial.print("connected client :");
			Serial.println(connected_socket_clients[i]);
			webSocket.sendTXT((uint8_t)connected_socket_clients[i],"test from slave");
		}
	}
}


void loop() {
	// if (!digitalRead(D8))
	// {
	//   start_sequence();
	// }

		MDNS.update();
	webSocket.loop();
	server.handleClient();
	// put your main code here, to run repeatedly:
}