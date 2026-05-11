// 시작
// D5~D8 - SD
// D1 SEN
// D2 REC
// D3 RELAY
// D4 unavailable
// A0 CDS

#include <Arduino.h>
#include <assert.h>

#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>

#include <SD.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


#define view 5
String myText[view];
String Array[view];
int index_ = 0;

const int RECV_PIN = D2; //an IR detector connected to D1
IRrecv irrecv(RECV_PIN);
decode_results results;

const int kIrLed = D1;  // ESP8266 GPIO pin to use. Recommended: 4 (D2)
IRsend irsend(kIrLed);

const int SD_CS = D8;
File dataFile;


//ESP8266WebServer server(80);
WiFiServer server(80);
String respond;

void wifiset()
{
    Serial.println("\n<wifiset>");
	const char* ssid = "ASUS_EA1B";//무선 공유기 id로 수정
	const char* password = "6000014475";//무선 공유기 비빌번호

	Serial.println('\n');
	Serial.print("Connecting to : ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(". ");
	}

	Serial.println("");
	Serial.println("WiFi connected");

	// Start the server
	server.begin();
	Serial.println("Server started");

	// Print the IP address
	Serial.println(WiFi.localIP());
}

void SDread()
{

	String sTemp[view];
	int index_ = 0;

	Serial.println("\n<SDcard>");
	while (!Serial) {}

	if (!SD.begin(D8))
	{
		Serial.println("SD card 연결 실패 !");
		return;
	}

	Serial.println("SD card 연결 성공 !");
	File dataFile = SD.open("save.txt");

	if (dataFile)
	{
		Serial.println("파일 열기 성공 !");
		int sd_index = 0;
		char sd;
		char sd_data[16];
		//String sd_data;

		while (dataFile.available())
		{
			sd = dataFile.read();

			if (sd == '\n')
			{ // 줄바꿈이면 다음 데이타로 인지
				//sd_data = "";
				if (index_ > view - 1)
				{
					for (int shift = 0; shift < view - 1; shift++)
						myText[shift] = myText[shift + 1];  // 10개 이상이면 배열 이동 저장
					myText[view - 1] = sd_data;
				}

				else
				{
					myText[index_] = sd_data;
				}
				sd_index = 0;
				index_++;

			}

			else
			{
				//Serial.println(sd);
				//sd_data += sd;
				sd_data[sd_index] = sd;
				sd_index++;
			}
		}

  }

  dataFile.close();
  Serial.println("SD 카드로 읽어 배열 변수에 저장한 데이타");
  
	for(int i=0; i<view; i++)
	{
	    Array[i] = myText[i];
        Serial.println(Array[i]);
	}
	    
	//for (int i = 0; i < view; i++)
        //Serial.println(myText[i]);
        
    //Serial.println();
    
}

void IRSD2()
{
	Serial.println("\n<IR+SD>");
	irrecv.resume(); // Receive the next value
  
	while (!Serial) {}

	if (!SD.begin(D8))
	{
		Serial.println("SD card 연결 실패 !");
		return;
	}

		Serial.println("SD card 연결 성공 !");
		dataFile = SD.open("save.txt", FILE_WRITE);
		dataFile.seek(EOF);// Append Mode
	
	if (dataFile)
	{
		Serial.println("파일 열기 성공 !");
		
		while (!irrecv.decode(&results))
		{
            //client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n got it!");
			delay(10);
		}
		
		if (irrecv.decode(&results))
		{
			Serial.println(results.value, HEX);
			dataFile.println(results.value, HEX);
			dataFile.flush();
			dataFile.close();
		}
	}
}

uint64_t getUInt64fromHex(String str)
{
    uint64_t accumulator = 0;
    for (size_t i = 0 ; isxdigit((unsigned char)str[i]) ; ++i)
    {
        char c = str[i];
        accumulator *= 16;
        if (isdigit(c)) /* '0' .. '9'*/
            accumulator += c - '0';
        else if (isupper(c)) /* 'A' .. 'F'*/
            accumulator += c - 'A' + 10;
        else /* 'a' .. 'f'*/
            accumulator += c - 'a' + 10;

    }

    return accumulator;
}

void IRsen(int k)
{
    Serial.println("\n<IRsend>");
	uint64_t buf = getUInt64fromHex(myText[k]);
	Serial.print("sent ");
	Serial.println(buf,HEX);
	irsend.sendNEC(buf);
}


void setup()
{
	Serial.begin(115200);
	Serial.println("시리얼 연결 중...");
	irsend.begin();
	Serial.println("IR발신 시작");
	irrecv.enableIRIn(); // Start the receiver
	Serial.println("IR수신 시작");
	delay(100);
	wifiset(); //start wifi setup
	pinMode(D3,1);

	SDread();
	delay(10);
}

void loop()
{

	// Check if a client has connected
	WiFiClient client = server.available();
	if (!client)
		return;

	// Wait until the client sends some data
	Serial.println("\nnew client");
	
	while (!client.available())
		delay(10);


	// Read the first line of the request
	String req = client.readStringUntil('\r');
	Serial.println(req);
	client.flush();

	// Match the request
	char val;
	if (req.indexOf("/get/") != -1)
		val = 'a';
	else if (req.indexOf("/A0/") != -1)
		val = 'b';
	else if (req.indexOf("/button0/") != -1)
		val = 'c';
	else if (req.indexOf("/button1/") != -1)
		val = 'd';
	else if (req.indexOf("/button2/") != -1)
		val = 'e';
	else if (req.indexOf("/button3/") != -1)
		val = 'f';
	else if (req.indexOf("/button4/") != -1)
		val = 'g';
	else if (req.indexOf("/swon/") != -1)
		val = 'h';
	else if (req.indexOf("/swoff/") != -1)
		val = 'i';
	else
	{
		Serial.println("invalid request");
		client.stop();
		return;
	}
	
	client.flush();

    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n ");

	switch (val)
	{
		case 'a':
			client.println("\nwaiting for input");
			IRSD2();
			delay(10);
			client.print("reloading");
			SDread();
			break;
		case 'b':
			Serial.println(analogRead(A0));
			client.print(analogRead(A0));
			client.print("/1024");
			break;
		case 'c':
			SDread();
			IRsen(0);
			client.print("\n0 has sent");
			break;
		case 'd':
			SDread();
			IRsen(1);
			client.print("\n1 has sent");
			break;
		case 'e':
			SDread();
			IRsen(2);
			client.print("\n2 has sent");
			break;
		case 'f':
			SDread();
			IRsen(3);
			client.print("\n3 has sent");
			break;
		case 'g':
			SDread();
			IRsen(4);
			client.print("\n4 has sent");
			break;
		case 'h':
			digitalWrite(D3,0);
			client.print("\nswitch0 on");
			break;
		case 'i':
			digitalWrite(D3,1);
			client.print("\nswitch0 off");
			break;
		default:
			// if nothing else matches, do the default
			// default is optional
			break;
	}

	delay(10);
	Serial.println("Client disonnected");
}
