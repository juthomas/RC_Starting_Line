#include <Arduino.h>
/**
 * ### PINOUT ###
 * 
 * Screen :
 * GND -> GND
 * VDD -> 5V
 * SCK -> D1  BLUE
 * SDA -> D2  GREEN
 * RES -> 5V
 * DC  -> GND
 * CS  -> GND
 * 
 * HC-SR04 Sensor :
 * VCC  -> 5V
 * TRIG -> D8  YELLOW
 * ECHO -> D0  GREEN
 * GND  -> GND
 * 
 * Lights :
 * BLACK  -> GND
 * RED    -> D5
 * ORANGE -> D6
 * GREEN  -> D7
 * 
 */

#include "user_interface.h"// for os_timer
#include "FS.h"// for open file on flash
//For scren
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WebSocketsServer.h>// Socket server library in 'lib' folder
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
// #define SENSOR_UPDATE 10
#define MIN_LAP_TIME 5000
// #define MAX_SENSOR_DISTANCE 40

int16_t g_sensorUpdate = 5;
int16_t g_sensorRefinement = 2;
int16_t g_maxSensorDistance = 40;

// #define SENSOR_REFINEMENT 3
#define SENSOR_REFINEMENT_MAX 20
bool sensorHistory[SENSOR_REFINEMENT_MAX] = {false};

os_timer_t myTimer;
os_timer_t startLightTimer;
const char* ssid = "RC_Startline";
const char* password = "88888888";
bool raceStarted = false;
bool lapTimeOn = true;
uint32_t raceStartTime = 0;
uint32_t lapStartTime = 0;
uint32_t lapTime = 0;

int8_t currentStartSequence = -1;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// SDA => D2
// SCK => D1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
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

#define TRIG D8
#define ECHO D0

int16_t connected_socket_clients[256] = {-1};

void fixedLoop(void *pArg);

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

void IRAM_ATTR startSequence(void)
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
		raceStarted = true;
		raceStartTime = millis();
		lapStartTime = millis();
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
		digitalWrite(D7, LOW);
		digitalWrite(D5, HIGH);
		timer1_attachInterrupt(startSequence);
		//Clock = 80 Mhz (80 000 000 Hz)
		timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
		//1 000 000 = 0.2s
		timer1_write(3750000); // 0.75s
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
			raceStarted = false;
			raceStartTime = 0;
			lapStartTime = 0;
			engageStart();
		}
		else if (strncmp((char*)payload, "[STOP]", sizeof("[STOP]") - 1) == 0)
		{
			if (raceStarted)
			{
				uint32_t raceTime = millis() - raceStartTime;
				char *socketMessage = (char*)malloc(sizeof(char) * 100);
				sprintf(socketMessage, "[RACETIME] %02d:%02d:%02d\n", raceTime / 60000 % 100, raceTime / 1000 % 60,raceTime % 1000);
				for (int16_t i = 0; i < 256; i++)
				{
					if (connected_socket_clients[i] != -1)
					{
						webSocket.sendTXT((uint8_t)connected_socket_clients[i], socketMessage);
					}
				}
				// for (int16_t i = 0; i < 256; i++)
				// {
				// 	if (connected_socket_clients[i] != -1)
				// 	{
				// 		webSocket.sendTXT((uint8_t)connected_socket_clients[i], ltoa(raceTime, socketMessage, 10));
				// 		webSocket.sendTXT((uint8_t)connected_socket_clients[i], "\n");
				// 	}
				// }
				free(socketMessage);
			}
			raceStarted = false;
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
		else if (strncmp((char*)payload, "[LAPTIME]", sizeof("[LAPTIME]") - 1) == 0)
		{
			if (strstr((char*)payload, "ON"))
			{
				lapTimeOn = true;
			}
			else if (strstr((char*)payload, "OFF"))
			{
				lapTimeOn = false;
			}
		}
		else if (strncmp((char*)payload, "[SETSENSORUPDATE]", sizeof("[SETSENSORUPDATE]") - 1) == 0)
		{
			// g_sensorUpdate = atoi((char*)payload + sizeof("[SETSENSORUPDATE]") - 1);
			Serial.printf("Sensor update value str:%s nu:%d\n", (char*)payload + sizeof("[SETSENSORUPDATE]") - 1, g_sensorUpdate);
		}
		else if (strncmp((char*)payload, "[SETREFINEMENT]", sizeof("[SETREFINEMENT]") - 1) == 0)
		{
			g_sensorRefinement = atoi((char*)payload + sizeof("[SETREFINEMENT]") - 1);
			// Serial.printf("Sensor update value str:%s nu:%d\n", (char*)payload + sizeof("[SETREFINEMENT]") - 1, g_sensorUpdate);
		}
		else if (strncmp((char*)payload, "[SETSENSORMAXDISTANCE]", sizeof("[SETSENSORMAXDISTANCE]") - 1) == 0)
		{
			g_maxSensorDistance = atoi((char*)payload + sizeof("[SETSENSORMAXDISTANCE]") - 1);
			// Serial.printf("Sensor update value str:%s nu:%d\n", (char*)payload + sizeof("[SETREFINEMENT]") - 1, g_sensorUpdate);
		}
		// Serial.println();
	}
}

void setup() {
	// WiFi.softAP(ssid, password);
	Serial.begin(115200);
	//display setup
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 1 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}
	display.setTextSize(1);	   // Normal 1:1 pixel scale
	display.setTextColor(WHITE); // Draw white text
	display.setCursor(0, 0);	   // Start at top-left corner
	display.cp437(true);
	display.setTextSize(3);

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
	server.serveStatic("/Applause.mp3", SPIFFS, "/Applause.mp3");
	server.serveStatic("/Race_Start.mp3", SPIFFS, "/Race_Start.mp3");
	server.serveStatic("/Begok.ttf", SPIFFS, "/Begok.ttf");
	server.serveStatic("/trackmania.ttf", SPIFFS, "/trackmania.ttf");



	server.begin();
	MDNS.addService("http", "tcp", 80);

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);

	pinMode(D5, OUTPUT);
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
	pinMode(ECHO, INPUT);
	pinMode(TRIG, OUTPUT);



	os_timer_setfn(&myTimer, fixedLoop, NULL);
	os_timer_arm(&myTimer, 1000, true);
	
	// put your setup code here, to run once:
}

void fixedLoop(void *pArg)// delay function didnt work in this scope
{
	
	// webSocket.sendTXT("test from slave");
	for (int16_t i = 0; i < 256; i++)
	{
		if (connected_socket_clients[i] != -1)
		{
			// Serial.print("connected client :");
			// Serial.println(connected_socket_clients[i]);
			// uint32_t currentRaceTime = millis() - raceStartTime;
			// char *itoaRet = (char*)malloc(sizeof(char) * 100);
			// webSocket.sendTXT((uint8_t)connected_socket_clients[i], ltoa(currentRaceTime, itoaRet, 10));
			// webSocket.sendTXT((uint8_t)connected_socket_clients[i], "\n");


		}
	}
}

uint32_t lastSensorUpdate = 0;


bool updateSensorHistory(bool currentState)
{
// #define g_sensorRefinement 5
// bool sensorHistory[g_sensorRefinement] = {false};
	bool tmp1;
	bool tmp2 = currentState;

	for (int i = 0; i < g_sensorRefinement; i++)
	{
		tmp1 = sensorHistory[i];
		sensorHistory[i] = tmp2;
		tmp2 = tmp1;
	}

	for (int i = 0; i < g_sensorRefinement; i++)
	{
		if (sensorHistory[i] == false)
		{
			return (false);
		}
	}
	return (true);
}

void loop() {
	// if (!digitalRead(D8))
	// {
	//   start_sequence();
	// }
	// Serial.println("In loop");

	// display_1.clearDisplay();
	if (raceStarted)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		uint32_t ttTimeD = millis() - raceStartTime;
		uint32_t lapTimeD = lapTimeOn ? millis() - lapStartTime : 0;
		display.printf("%03d:%03d\n", ttTimeD / 1000 % 1000, ttTimeD % 1000);
		display.setCursor(0, 40);
		display.printf("%03d:%03d\n", lapTimeD / 1000 % 1000, lapTimeD % 1000);
		display.display();
	}
	else
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		uint32_t ttTimeD = 0;//raceStartTime;
		uint32_t lapTimeD = 0;//lapStartTime;
		display.printf("%03d:%03d\n", ttTimeD / 1000 % 1000, ttTimeD % 1000);
		display.setCursor(0, 40);
		display.printf("%03d:%03d\n", lapTimeD / 1000 % 1000, lapTimeD % 1000);
		display.display();
	}

	if (raceStarted && millis() - lastSensorUpdate > g_sensorUpdate)
	{
		long duration;
		int distance;
		lastSensorUpdate = millis();
		digitalWrite(TRIG, LOW);
		delayMicroseconds(2);
		digitalWrite(TRIG, HIGH);
		delayMicroseconds(10);
		digitalWrite(TRIG, LOW);
		duration = pulseIn(ECHO, HIGH, 500000);

		// Calculating the distance
		distance = duration*0.034/2;
		// Prints the distance on the Serial Monitor
		Serial.print("Distance: ");
		Serial.println(distance);
		
		if (updateSensorHistory(distance < g_maxSensorDistance && ((lapTimeOn && millis() - lapStartTime > MIN_LAP_TIME) || (!lapTimeOn && millis() - lapStartTime > 100 &&  millis() - raceStartTime > MIN_LAP_TIME) )))
		{


			lapTime = millis() - lapStartTime;
			lapStartTime = millis();
			char *socketMessage = (char*)malloc(sizeof(char) * 100);

			if (lapTimeOn)
			{
				sprintf(socketMessage, "[LAPTIME] %02d:%02d:%02d\n", lapTime / 60000 % 100, lapTime / 1000 % 60,lapTime % 1000);
				for (int16_t i = 0; i < 256; i++)
				{
					if (connected_socket_clients[i] != -1)
					{
						// webSocket.sendTXT((uint8_t)connected_socket_clients[i], ltoa(lapTime, socketMessage, 10));
						webSocket.sendTXT((uint8_t)connected_socket_clients[i], socketMessage);
					}
				}
			}
			else
			{
				sprintf(socketMessage, "[LAPSOUND]\n");
				for (int16_t i = 0; i < 256; i++)
				{
					if (connected_socket_clients[i] != -1)
					{
						webSocket.sendTXT((uint8_t)connected_socket_clients[i], socketMessage);
					}
				}
			}
			free(socketMessage);
			// requestSendLapTime = true;
		}
		Serial.println(GET_FREE_HEAP);


	}


	MDNS.update();
	webSocket.loop();
	server.handleClient();
	// put your main code here, to run repeatedly:
}