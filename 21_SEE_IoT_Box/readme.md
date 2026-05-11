# [IoT Box] Wi-Fi 기반 만능 리모컨 및 센서 제어 시스템

### 발표자료
- **시연동영상:** [Youtube](https://youtu.be/qPJH8z3dYJI?si=QT03M3E3cImV89N8)

---

## 1. 프로젝트 개요
ESP8266(NodeMCU)을 메인 컨트롤러로 사용하여, 적외선(IR) 신호를 학습 및 송신하고 다양한 센서 데이터를 웹을 통해 모니터링할 수 있는 **IoT 허브 시스템**입니다. 기존 가전제품의 리모컨 신호를 SD 카드에 저장해두었다가 필요할 때 웹 버튼 클릭 한 번으로 송신하여 가전제품을 제어할 수 있습니다.

---

## 2. 주요 기능
- **IR Remote Learning & Sending:**
  - **수신:** 리모컨의 IR 신호를 수신하여 16진수 값으로 분석.
  - **저장:** 분석된 IR 데이터를 SD 카드의 `save.txt` 파일에 기록하여 전원이 꺼져도 유지.
  - **송신:** 저장된 데이터를 읽어와 가전제품(TV, 에어컨 등)에 송신(NEC 프로토콜 기반).
- **Web Server Interface:**
  - 웹 브라우저를 통해 접속하여 버튼 클릭으로 기기 제어.
  - 조도 센서(CDS) 값 실시간 모니터링.
  - 릴레이 스위치(D3) On/Off 제어.
- **SD Card Data Management:**
  - SPI 통신을 통해 대량의 리모컨 신호를 관리하고 필요 시 실시간으로 리스트 갱신.

---

## 3. 하드웨어 구성
- **MCU:** ESP8266 (Wi-Fi 모듈 내장)
- **적외선 통신:** IR Receiver (D2), IR LED Sender (D1)
- **저장 장치:** SD Card Module (SPI: D5~D8)
- **센서 및 제어:** 조도 센서 (A0), Relay 모듈 (D3)
- **상태 표시:** Serial Monitor (115200bps)

---

## 4. API 가이드 (HTTP Request)
- `/get/`: 새로운 IR 신호 수신 모드 진입 및 SD 카드 저장.
- `/A0/`: 현재 조도 센서 값 반환.
- `/button[0-4]/`: SD 카드에 저장된 n번째 IR 신호 송신.
- `/swon/`, `/swoff/`: 릴레이 스위치 제어.
