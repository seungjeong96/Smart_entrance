#define PIR_1 15
#define PIR_2 26
#define LED_1 32
#define Buzzer 2
#define LIGHT 36
#define HUMTEMP 33 //핀 번호 Define

#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <DHT.h> //온습도 센서 활용을 위한 DHT 라이브러리 로드

DHT dht(HUMTEMP, DHT11); //HUMTEMP 핀에 DHT11 모듈이 연결 될 것

int freq = 2000; // PWM 주파수 설정(2000Hz)
int channel = 0; // PWM 채널 설정(0~15)
int resolution = 8; // PWM duty cycle resolution(해상도) 설정, 8bit=0~255
int currentWeather = 0; //1:맑음, 2:흐림, 3:비, 4:눈
int lightThreshold = 1300; //우산 감지 센서 (조도 센서) 인식 기준 값
int insideTemp; //실내 온도
int insideHumi; //실내 습도

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731(); // LED 매트릭스를 통해 해, 구름, 비, 눈, 스마일, 찡그림 이모티콘 구현
static const uint8_t PROGMEM
sunny_bmp[] = {
  B11110111,
  B11110000,
  B11110110,
  B11100001,
  B00001000,
  B10100100,
  B10100010,
  B10010000,
  B00000000
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
  B00001000,
  B00000000
};
static const uint8_t PROGMEM
cloudy_bmp[] = {
  B00000000,
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
  B01010100,
  B01010100
};
static const uint8_t PROGMEM
umbrella_bmp[] = {
  B00011000,
  B01111110,
  B01111110,
  B11111111,
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
  B00101010,
  B00000000
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
  B00000000,
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
  B00000000,
  B00000000
};

static const uint8_t PROGMEM
check_bmp[] = {
  B00000000,
  B00000000,
  B00000010,
  B00000110,
  B01101100,
  B01111000,
  B00110000,
  B00000000
};

const char text[7] = {'W', 'E', 'L', 'C', 'O', 'M', 'E'}; // 사용자가 집으로 돌아올 때 출력되는 문구 지정

void setup() {
  pinMode(PIR_1, INPUT); //D1 1번 PIR
  pinMode(PIR_2, INPUT); //D2 2번 PIR
  pinMode(LED_1, OUTPUT); //D3 LED 조명
  pinMode(LIGHT, INPUT); //A0 우산 휴대 감지기

  ledcSetup(channel, freq, resolution); // PWM 채널, 주파수, 해상도 설정(ledcSetup)
  ledcAttachPin(25, channel); // 설정한 PWM 채널의 출력을 어떤 핀(25)으로 내보낼 지 지정
  pinMode(Buzzer, OUTPUT); // 2번 핀을 출력핀으로 지정하고 0V로 구동되도록 각 pin에 LOW 값 지정
  //pinMode(25, OUTPUT);//2번만 전원, 25는 단순 핀 연결 : 핀 지정 필요 X
  digitalWrite(Buzzer, LOW);

  matrix.begin(); // I2C주소값 선언, begin(uint8_t addr = ISSI_ADDR_DEFAULT); I2c버스로 해당 드라이버를 사용할 수 있도록 준비함
  // #define ISSI_ADDR_DEFAULT 0x74
  Serial.begin(115200);

  dht.begin(); //온습도센서 begin
}

void printWelcome() {
  matrix.clear(); // 기존 매트릭스 값 clear
  for (int j = 7; j >= -40; j--) { // 텍스트가 우측에서 출력되며 나와 좌측으로 들어가며 사라지도록 for문 지정
    //j값은 LEDM Matrix의 X좌표로 사용할 것
    matrix.clear(); // LED Matrix 초기화, 해당 함수가 없을 경우, 한번 켜진 LED가 꺼지지 않아 단어가 지나간 자리에 잔상이 남게됨
    matrix.setCursor(j, 1); // LED Matrix 출력의 좌표값(x, y): matrix.setCursor() 함수
    // for문이 진행되며 해당 함수로 인해 조금씩 단어의 위치가 바뀌게 됨
    matrix.printf("%s", text);// 문자 배열값 출력(%c=문자1개) : matrix.printf() 함수
    // AJOU 네 글자가 모두 출력되도록 %c를 string을 관리할 때 사용되는 %s로 변경

    delay(70);
    //좌측 맨 아래 줄이 1,1
  }
  matrix.clear();
}

int weatherLEDmatrix(int status) {

  matrix.setRotation(0); // 디스플레이 위치 값
  matrix.clear(); // LED Matirx 화면 초기화
  // matrix.drawBitmap(float x(x좌표값), float y(y좌표값), text, int width(가로행), int height(세로열), brightness(밝기))
  if (status == 1) { //맑음
    //matrix.clear();
    matrix.drawBitmap(float(0), float(0), sunny2_bmp, 8, 9, 100);
    // 형식을 맞춰주기 위해 float()사용, 가로 행에는 LED Matrix의 가로 LED 해상도를, 세로 열에는 세로 해상도를 입력
  }
  else if (status == 2) { //흐림
    //matrix.clear();
    matrix.drawBitmap(float(0), float(0), cloudy_bmp, 8, 9, 100);
  }
  else if (status == 3) { //비 or 우산 or 태풍
    while (1) { // 우산을 들고 나가야 하는 경우라면 우산 아이콘과 비 아이콘을 번갈아 표시
      matrix.drawBitmap(float(0), float(0), rainy_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      matrix.drawBitmap(float(0), float(0), umbrella_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      if (analogRead(36) < lightThreshold) { //만약 조도센서의 값이 기준치 보다 낮아질 경우, 다시 말해 사용자가 우산을 가져가 조도 센서 앞에 빛을 가리고 있는 우산이 사라진 경우
        break; //while 문을 끝내고
      }
    }
    matrix.drawBitmap(float(0), float(0), check_bmp, 8, 9, 100); //체크 아이콘을 표시
    nonUmbrellaSound(); // 수행 완료 음악 재생 (일반 날씨 사운드와 동일한 음악 사용)
  }
  else if (status == 4) { //눈, 앞서 살펴본 비와 동일한 메커니즘으로 동작
    while (1) {
      matrix.clear();
      matrix.drawBitmap(float(0), float(0), snow_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      matrix.drawBitmap(float(0), float(0), umbrella_bmp, 8, 9, 100);
      delay(2000);
      matrix.clear();
      if (analogRead(36) < lightThreshold) {
        break;
      }
    }
    matrix.drawBitmap(float(0), float(0), check_bmp, 8, 9, 100);
    nonUmbrellaSound();
  }
  else { //만일 날씨 값이 수신되지 않았다면 스마일 표시
    matrix.drawBitmap(float(0), float(0), smile_bmp, 8, 9, 100);
  }
}

void umbrellaSound() { // 버저를 통해 사용자가 현관을 지날 때 (집을 들어올 때, 나갈 때, 우산이 필요할 때 등) 나오는 소리 멜로디 세팅
  digitalWrite(Buzzer, HIGH); //버저를 켜고
  ledcWriteTone(channel, 261); // 도
  delay(200);
  ledcWriteTone(channel, 261); // 도
  delay(200);
  ledcWriteTone(channel, 261); // 도
  delay(200);
  ledcWriteTone(channel, 392); // 솔
  delay(400);
  digitalWrite(Buzzer, LOW); //소리 출력후 버저 OFF
}

void nonUmbrellaSound() { //솔 미 파 레 도 (높은)도
  digitalWrite(Buzzer, HIGH);
  ledcWriteTone(channel, 392);
  delay(200);
  ledcWriteTone(channel, 329);
  delay(200);
  ledcWriteTone(channel, 349);
  delay(200);
  ledcWriteTone(channel, 293);
  delay(200);
  ledcWriteTone(channel, 261);
  delay(400);
  ledcWriteTone(channel, 523);
  delay(400);
  digitalWrite(Buzzer, LOW);
}

void welcomeSound() { //도 미 솔 시 (높은) 도
  digitalWrite(Buzzer, HIGH);
  ledcWriteTone(channel, 261);
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

int THIchecker() { //불쾌 지수 계산
  int calc;
  float temp, humi;
  temp = dht.readTemperature(); //온도 값 read
  humi = dht.readHumidity(); // 습도 값 read
  calc = 0.81 * temp + 0.01 * humi * (0.99 * temp - 14.3) + 46.3;
  //불쾌지수 계산
  Serial.print(temp);
  Serial.print(",");
  Serial.print(humi);
  Serial.print(",");
  Serial.println(calc);
  //Node-RED에서 값을 읽을 수 있도록 값 출력
  //하지만 Node-RED에서 동일한 포트에서 데이터 입 출력을 동시에 진행하지 못해 해당 기능 구현 실패
  return calc;
}

void loop() {
  int insideSensor = digitalRead(PIR_1); //1번 PIR 센서를 현관문 쪽
  int doorsideSensor = digitalRead(PIR_2); // 2번 PIR 센서를 집 안쪽에 위치
  int inOrOut = 0; // 들어옴 : 1 , 나감 : 2

  if (Serial.available()) {
    currentWeather = Serial.parseInt(); //Node-RED로 부터 현재 날씨에 상응하는 숫자를 입력 받음
  }

  if (insideSensor == 1 || doorsideSensor == 1) { //만약 두 PIR 센서 중 하나라도 값이 들어왔을 때 현관등(LED)ON
    digitalWrite(LED_1, HIGH);
  }

  if (insideSensor == 1 && doorsideSensor == 0) {
    //만약 안쪽의 PIR이 먼저 인식되었을 경우
    //사용자가 나가는 것으로 인식
    if (currentWeather == 3 || currentWeather == 4) { //우산이 필요한 경우
      umbrellaSound(); //우산 필요 멜로디 재생
    }
    else { //우산이 필요없는 경우
      nonUmbrellaSound(); // 일반 멜로디 재생
    }
    weatherLEDmatrix(currentWeather); //현재 날씨 표시
    delay(10000);
    matrix.clear(); //10초 후 클리어
    insideSensor == 0;
  }
  else if (insideSensor == 0 && doorsideSensor == 1) {
    //만약 현관문쪽의 PIR이 먼저 인식되었을 경우
    // 사용자가 들어오는 것으로 인식
    // Serial.println("들어옴");
    
    welcomeSound(); //웰컴 사운드 재생
    printWelcome(); //Welcome 글씨를 LED Matrix로 출력
    
    int THI = THIchecker(); //불쾌 지수 계산
    //Serial.println(THI);
    if (THI > 80) { //만약 불쾌지수가 높다면 화난 아이콘을
      matrix.drawBitmap(float(0), float(0), angry_bmp, 8, 9, 100);
    }
    else { //그렇지 않다면 웃는 아이콘을 LED MATRIX에 표시
      matrix.drawBitmap(float(0), float(0), smile_bmp, 8, 9, 100);
    }
    
    delay(10000);
    matrix.clear(); //100초 뒤 LED MATRIX 클리어
    doorsideSensor == 0;
  }


  else {
    Serial.println("출입 없음"); //만약 출입이 없다면
    digitalWrite(LED_1, LOW); //LED 등을 끄고
    //THIchecker(); //THI CHECKER를 통해 실내 온습도 및 불쾌지수 지속 수집 (NODE-RED 문제로 구현 불가)
    matrix.clear(); //LED Matrix도 클리어
  }

}