# [AIIoTESG] AI와 IoT를 활용한 에너지 절약 ESG 시스템

### 발표자료
- **시연동영상:** [Youtube](https://youtu.be/AbdshAkhsO4?si=ZLpSlKuI4k0c3SnW)
- **보고서:** [PDF](./2025년%20한이음%20드림업%20개발보고서.pdf)
- **포스터:** [PDF](./[7조]%20AI와%20IoT를%20활용한%20에너지%20절약%20ESG시스템%20구축.pdf)

---

## 1. 프로젝트 소개
**"데이터 기반의 지능형 에너지 관리와 ESG 경영의 결합"**
본 프로젝트는 IoT 플랫폼을 통해 실시간 전력 데이터를 수집하고, AI를 사용하여 사용 패턴을 분석하고, 최적의 에너지 절약 방안을 제시하는 시스템입니다. 개인에게는 전기 요금 절감과 편의성을, 기업에게는 탄소 배출량 관리 및 ESG 보고서 자동 생성 기능을 제공하는 것을 목표로 합니다.

---

## 2. 주요 기능
- IOT
  - **실시간 모니터링 및 이상 탐지:** 과부하, 누전 등 이상 징후를 즉시 감지하여 안전사고 예방.
  - **원격 제어 및 자동화:** MQTT 통신을 이용한 가전/조명 원격 On/Off 및 센서 기반 자동 제어.
- AI
  - **AI 기반 전력 사용량 분석:** 전력 사용 패턴을 바탕으로 제한된 측정 장비 환경에서 과사용 되는 기기 추론
  - **에너지 사용 시간대 추천:** 시간대별 요금을 분석하여 가장 경제적인 고전력 기기 운용 시점 제안.
  - **ESG 보고서 자동 생성:** 전력 사용량과 탄소 배출량을 통합 관리하고, Claude API를 통해 기업 맞춤형 ESG 보고서 출력.

---

## 3. 시스템 아키텍처 및 기술 스택

### 3.1. 하드웨어 구성 (IoT Side)
- **Main MCU:** ESP32 (FreeRTOS 기반 ESP-IDF 적용)
- **Sensors:**
  - **전력 측정:** PZEM-004T (실시간 전류/전압 감지)
  - **환경 측정:** DHT22 (온습도), CDS (조도)
- **Control:** 릴레이 모듈 (직접 전원 제어)
- **Communication:** Wi-Fi, MQTT

### 3.2. 소프트웨어 스택 (Server Side)
- **github:** [repository](https://github.com/yeop-sang/hanium_power_monitor_server)
- **Backend:** Python (Flask), MySQL (Data Logging)
- **AI/ML:** Pytorch (소비 패턴 분석), CNN, Claude API (보고서 자동 생성)
- **Frontend:** HTML/CSS/JS, Vue.js
- **Infrastructure:** Docker, Cuda, wireguard VPN
---