# Smart Entrance System

## Description

ESP32를 이용한 스마트 현관 시스템 프로젝트로, 사용자가 입출입 여부를 확인하여, 외출시 현재 지역의 날씨와 우산 필요 여부를, 반대로 사용자가 집으로 들어오는 경우 실내의 불쾌지수를 표시한다.

## Prerequisite

1. Arduino IDE

- Adafruit_GFX.h
- Adafruit_IS31FL3731.h
- DHT.h

2. Node-RED

- node-red-dashboard
- node-red-node-serialport
- node-red-node-openweathermap

## Devices

1. Kepler ESP-A (ESP32 included)
2. Kepler ESP-A I/O Cube Extension
3. PIR Sensor Module \* 2
4. LED Module
5. CdS Brightness Sensor Module
6. Temperature / Humidity Sensor Module (dht 11)

## 3. 타겟 시뮬레이션

### Node-RED

1. Openweathermap에서 날씨 정보를 지속적으로 수집한다.
2. 수집된 데이터 중 날씨(weather), 지역(location), 기온(tempc), 습도(humidity) 를 따로 저장한다.
3. weather 정보를 지정한 숫자에 맞게 변환하고, 이를 Serial 포트로 내보낸다.
4. Node-RED 대시보드에 날씨, 지역, 기온, 습도 정보와 날씨 정보를 기반으로 우산 필요 여부를 표시한다. 이 때 기온과 습도 정보는 게이지 UI를 활용하여 표시한다.

### Kepler ESP-A

1. Node-RED에서부터 숫자로 변환된 날씨 데이터를 수신하여, 현재 날씨가 어떤지 확인한다.
2. 두개의 PIR 센서 중 하나라도 동작할 경우 현관등의 LED 등을 켠다.

#### 사용자 외출 시

##### 2개의 PIR Sensor 중 현관문으로부터 먼 센서가 먼저 동작할 경우

3. 맑음, 흐림등과 같이 우산이 필요 없는 날씨라면 간단한 멜로디와 함께 날씨 이모티콘을 LED Matrix에 표시한다.
4. 비, 눈, 태풍 등과 같이 우산이 필요한 날씨라면 알림음 멜로디를 출력하고, 날씨 아이콘과 우산 아이콘을 번갈아 가며 표시한다. 만약 조도센서를 통해 사용자가 우산을 가져간것이 확인될 경우, 반복을 중단하고 간단한 멜로디와 함께 체크 표시 아이콘을 LED Matrix를 통해 출력한다.

#### 사용자 귀가 시

##### 2개의 PIR Sensor 중 현관문으로부터 가까운 센서가 먼저 동작할 경우

3. 웰컴 사운드와 함께 Welcome 글자를 LED Matrix를 통해 출력한다.
4. 실내의 온도와 습도 데이터를 받아온 다음, 이를 기반으로 불쾌지수를 계산한다.
5. 불쾌지수가 80 이상이라면 화난 아이콘을, 그렇지 않다면 웃는 아이콘을 LED Matrix를 통해 출력한다.
