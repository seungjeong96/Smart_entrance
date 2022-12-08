#include <WiFiUdp.h>
#include <WiFi.h>
#include <SparkFunBME280.h>
#define BME280_ADDR 0x77
const char* ssid = "Ajou Univ";
const char* password = "";

#define serverIP "172.20.10.8"
#define udpPort 3456

BME280 myBME280;
WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  myBME280.setI2CAddress(BME280_ADDR); // BME280 I2C주소값 설정(0x77)
  myBME280.beginI2C(); // BME280 I2C장치 호출
  pinMode(15, OUTPUT); // LED(15) pin mode 설정
  pinMode(2, INPUT); // 버튼(2) pin mode 설정

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {//와이파이가 연결될때 까지
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  udp.begin(udpPort); //해당 포트 번호의 UDP 인스턴스 Initialize
}

int insideTempAndHumi(int temp, int humi){
  double calc;
  calc = 0.81*temp+0.01*humi*(0.99*temp-14.3)+46.3;
  if(calc>80){
    return 1;
  }
  else{
    return 0;;
  }
}

void loop() {

  udp.beginPacket(serverIP, udpPort);
  // 해당 IP와 포트번호에 열린 다른 UDP 인스턴스와 연결을 시작
  float temp = myBME280.readTempC();
  float humi = myBME280.readFloatHumidity();
  String insideTempHumi = "온도:" + String(temp) + "C,습도: " + String(humi) + "%";
  // int buttonState = digitalRead(2); //현재 버튼 상태 Read
  // Serial.print("Button Status: ");
  int THI = insideTempAndHumi(temp,humi);
  Serial.println(insideTempHumi);// 버튼 상태 출력 후
  udp.print(THI); // 상대 UDP Instance에게 ASCII 데이터 Write
  udp.endPacket(); //데이터 전송 성공시 1 리턴

  delay(500);


}