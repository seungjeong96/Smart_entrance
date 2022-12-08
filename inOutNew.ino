#define PIR_1 15
#define PIR_2 26
#define LED_1 32
#define Buzzer 2

#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <WiFi.h>
#include <WiFiUdp.h>
const char* ssid = "Ajou Univ";
const char* password = "";
WiFiServer server(80);
WiFiUDP Udp;

int freq = 2000; // PWM 주파수 설정(2000Hz)
int channel = 0; // PWM 채널 설정(0~15)
int resolution = 8; // PWM duty cycle resolution(해상도) 설정, 8bit=0~255
int currentWeather = 0; //1:맑음, 2:흐림, 3:비, 4:눈
unsigned int localUdpPort = 3456; //사용할 UDP 포트 번호 지정
char ReceivedMessage[255]; //수신할 메시지를 저장할 배열 공간 선언
char Acknowledge[] = "Received OK";

int insideTemp;
int insideHumi;

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();
static const uint8_t PROGMEM
sunny_bmp[] = {
  B11110111,
  B11110000,
  B11110110,
  B11100001,
  B00001000,
  B10100100,
  B10100010,
  B10010000
};
static const uint8_t PROGMEM
sunny2_bmp[] = {
  B00000000,
  B00001000,
  B00101010,
  B00011100,
  B01110111,
  B00011100,
  B00101010,
  B00001000
};
static const uint8_t PROGMEM
cloudy_bmp[] = {
  B00000000,
  B00001000,
  B00111100,
  B01100010,
  B11000111,
  B01111100,
  B00000000,
  B00000000

};
static const uint8_t PROGMEM
rainy_bmp[] = {
  B00111100,
  B01111110,
  B11111111,
  B01111100,
  B00000000,
  B01010100,
  B01010100,
  B00000000
};
static const uint8_t PROGMEM
umbrella_bmp[] = {
  B00011000,
  B01111110,
  B01111110,
  B11111111,
  B00001000,
  B00001000,
  B00101000,
  B00111000
};
static const uint8_t PROGMEM
snow_bmp[] = {
  B00111100,
  B01111110,
  B11111111,
  B01111110,
  B00000000,
  B01010100,
  B00000000,
  B10101000
};
static const uint8_t PROGMEM
smile_bmp[] = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B01000010,
  B01100110,
  B00111100,
  B00000000
};
static const uint8_t PROGMEM
angry_bmp[] = {
  B00000000,
  B01100110,
  B01100110,
  B00000000,
  B00111100,
  B01100110,
  B01000010,
  B00000000
};

void setup() {
  pinMode(PIR_1, INPUT); //D1 1번 PIR
  pinMode(PIR_2, INPUT); //D2 2번 PIR
  pinMode(LED_1, OUTPUT); //D3 LED 조명

  ledcSetup(channel, freq, resolution); // PWM 채널, 주파수, 해상도 설정(ledcSetup)
  ledcAttachPin(25, channel); // 설정한 PWM 채널의 출력을 어떤 핀(25)으로 내보낼 지 지정
  pinMode(Buzzer, OUTPUT); // 2번 핀을 출력핀으로 지정하고 0V로 구동되도록 각 pin에 LOW 값 지정
  //pinMode(25, OUTPUT); // 부저 전원인가를 위한 GPIO 제어핀 제어
  //2번만 전원, 25는 단순 핀 연결 : 핀 지정 필요 X
  digitalWrite(Buzzer, LOW); // 부저 전원 ON



  matrix.begin(); // I2C주소값 선언, begin(uint8_t addr = ISSI_ADDR_DEFAULT); I2c버스로 해당 드라이버를 사용할 수 있도록 준비함
  // #define ISSI_ADDR_DEFAULT 0x74
  Serial.begin(115200);


  //  Serial.println();
  //  Serial.print("Connecting to ");
  //  Serial.println(ssid);
  //  WiFi.begin(ssid, password);
  //  while (WiFi.status() != WL_CONNECTED) { //와이파이가 연결될때 까지
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  Serial.println("");
  //  Serial.println("WiFi connected.");
  //  Serial.println("IP address: ");
  //  Serial.println(WiFi.localIP()); //와이파이 연결 시 IP 주소 출력
  //  Udp.begin(localUdpPort); //해당 포트 번호의 UDP 인스턴스 Initialize
  //  Serial.printf("I am listening at IP %s, UDP port %d \n", WiFi.localIP().toString().c_str(), localUdpPort);
  //  //현재 연결된 와이파이 IP 주소 및 UDP 포트 출력




}


int weatherLEDmatrix(int status) {

  matrix.setRotation(0); // 디스플레이 위치 값
  matrix.clear(); // LED Matirx 화면 초기화
  // matrix.drawBitmap(float x(x좌표값), float y(y좌표값), text, int width(가로행), int height(세로열), brightness(밝기))

  if (status == 1) { //맑음
    matrix.drawBitmap(float(0), float(0), sunny_bmp, 8, 9, 100);
    // 형식을 맞춰주기 위해 float()사용, 가로 행에는 LED Matrix의 가로 LED 해상도를, 세로 열에는 세로 해상도를 입력
  }
  if (status == 2) { //흐림
    matrix.drawBitmap(float(0), float(0), cloudy_bmp, 8, 9, 100);
  }
  if (status == 3) { //비 or 우산 or 태풍

    for (int displayCount = 0; displayCount < 5; displayCount++) {
      matrix.drawBitmap(float(0), float(0), rainy_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      matrix.drawBitmap(float(0), float(0), umbrella_bmp, 8, 9, 100);
      delay(2000);
    }
  }  
  if(status == 4){//눈
      for (int displayCount = 0; displayCount < 5; displayCount++) {
      matrix.drawBitmap(float(0), float(0), snow_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      matrix.drawBitmap(float(0), float(0), umbrella_bmp, 8, 9, 100);
      delay(2000);
    }
  }
  else{
  matrix.drawBitmap(float(0), float(0), sunny_bmp, 8, 9, 100);
  }
}

void umbrellaSound() {
  digitalWrite(Buzzer, HIGH);
  ledcWriteTone(channel, 261);
  delay(100);
  ledcWriteTone(channel, 261);
  delay(100);
  ledcWriteTone(channel, 261);
  delay(100);
  ledcWriteTone(channel, 392);
  delay(200);
  digitalWrite(Buzzer, LOW);
}

void nonUmbrellaSound() {
  digitalWrite(Buzzer, HIGH);
  ledcWriteTone(channel, 392);
  delay(200);
  ledcWriteTone(channel, 329);
  delay(200);
  ledcWriteTone(channel, 349);
  delay(200);
  ledcWriteTone(channel, 293);
  delay(200);
  ledcWriteTone(channel, 261); // 부저의 PWM 주파수 기준 값, C(도) 261.6256
  delay(200);
  ledcWriteTone(channel, 523);
  delay(200);
  digitalWrite(Buzzer, LOW);
}

void welcomeSound() {
  digitalWrite(Buzzer, HIGH);
  ledcWriteTone(channel, 261); // 부저의 PWM 주파수 기준 값, C(도) 261.6256
  delay(200);
  ledcWriteTone(channel, 329);
  delay(200);
  ledcWriteTone(channel, 392);
  delay(200);
  ledcWriteTone(channel, 493);
  delay(200);
  ledcWriteTone(channel, 523);
  delay(200);
  digitalWrite(Buzzer, LOW);
}

int insideTempAndHumi(int temp, int humi){
  int calc;
  calc = 0.81*temp+0.01*humi*(0.99*temp-14.3)+46.3;
  return calc;
}

void loop() {

  int motionStatusOfDoorside = digitalRead(PIR_1);
  int motionStatusOfInside = digitalRead(PIR_2);
  int inOrOut = 0; // 들어옴 : 1 , 나감 : 2

  if (Serial.available()) {
    currentWeather = Serial.parseInt();
  }
  
  if (motionStatusOfDoorside == 1 || motionStatusOfInside == 1) {
    digitalWrite(LED_1, HIGH);
  }



  if (motionStatusOfDoorside == 1 && motionStatusOfInside == 0) {
    Serial.println("나감");
    Serial.println("오늘의 날씨는");

    if (currentWeather == 3 || currentWeather == 4) { //우산이 필요한 경우
      umbrellaSound();
    }
    else { //우산이 필요없는 경우
      nonUmbrellaSound();
    }
    weatherLEDmatrix(currentWeather);
    delay(10000);
    motionStatusOfDoorside == 0;
  }
else if (motionStatusOfDoorside == 0 && motionStatusOfInside == 1) {
    Serial.println("들어옴");
    welcomeSound();

    Udp.flush(); //버퍼 clear
    Serial.print("parse :");
    Serial.println(Udp.parsePacket()); //수신된 데이터를 번역
    Udp.read(ReceivedMessage, 255); // 버퍼에 있는 데이터를 읽어들임
    //udp read의 반환값 :TRUE FALSE
    //근데 read 반환값을 프린트에 넣어버림

    Serial.println("현재 실내 온도는");
    Serial.println(ReceivedMessage); //번역된 메시지를 출력

    char THI = ReceivedMessage[0];
    if(THI ='1'){
      matrix.drawBitmap(float(0), float(0), angry_bmp, 8, 9, 100);
    }
    else{
      matrix.drawBitmap(float(0), float(0), smile_bmp, 8, 9, 100);
    }
    
    delay(10000);
    
    motionStatusOfInside == 0;
  }


  else {
    Serial.println("출입 없음");
    digitalWrite(LED_1, LOW);
    matrix.clear();
  }

}
