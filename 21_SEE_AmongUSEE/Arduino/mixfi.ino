#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_Fingerprint.h>
#include <MsTimer2.h>

String sid = "00000000"; //학번

// 핀
const int BT1 = 13;
const int BT2 = 12;
const int BT3 = 11;
const int BT4 = 10;

const int data  = 2;
const int latch = 3;
const int clk   = 4;

const int ledR  = 6;
const int ledG  = 5;

//부저
const int buzzerPin = A0;

// button
int btPress;   // btPress : 활성화

// buzzer
int bzCon; 

// lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);
int lcdNum = -1;  // lcdNum: 첫시작숫자
int lcdLoc = 7;   // lcdLoc : 칸 자리
String lcdID = "";
int lcdBlink, lcdSidT, lcdSidF, lcdFinT, lcdFinF, lcdsidFF, lcdfidFF, lcdFinW; //student id True False, Fingerprint input True False

// temperature
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
int temCon,temCount;
float temBuf,temData;

// fingerpreint
SoftwareSerial mySerial(A1, A2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int FP, timeout;
uint8_t id;
String Data1;


//light
const int infrared  = A3;
int irT;
String serbuf = "";
int Fid;
String sidre, timere;

void setup() {
    Serial.begin(9600);

    pinMode(buzzerPin, OUTPUT);
    pinMode(BT1, INPUT_PULLUP);   // bt1 : 숫자가 1만큼 증가
    pinMode(BT2, INPUT_PULLUP);   // bt2 : 커서 다음칸으로 넘아가기
    pinMode(BT3, INPUT_PULLUP);   // bt3 : 숫자지우면서 이전칸으로 넘어가기
    pinMode(BT4, INPUT_PULLUP);   // bt4 : 확인
    pinMode(infrared, INPUT);

    pinMode(latch, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(data, OUTPUT);
    

    pinMode(ledR, OUTPUT);
    pinMode(ledG, OUTPUT);

    MsTimer2::set(1000, ledFlash); // 500ms period
    MsTimer2::start();

    mlx.begin();
    fingerinit();

    lcd.begin();  //lcd시
    lcd.backlight();  //lcd 파란불
    lcd.setCursor(0, 0);    //커서 첫번째줄 첫번째칸 위치
    lcd.print("NUMBER:");   //lcd에 ()나타내
}

void loop() { 
    light();      
    button();
    buzzer();
    temperature();
    screen(); 
    fingerprint();
    ser();
    delay(10);
}
