# DSD 2025 - Final Project Report: Reconfigurable FIR Filter

### 발표자료
- **보고서**: [PDF](./DSD2025_Final_Report.pdf)

---

## 1. 최종 구현 시스템 개요
본 시스템은 4-Parallel SRAM 구조를 가진 재구성 가능한(Reconfigurable) FIR 필터입니다.

### 1.1. FIR 필터 기본 사양
- **사양 지시서**: [PDF](./FIR%20filter%20with%20kaiser%20window_400kHz%20BW,%20600kHz%20Sampling,%20200kHz%20Symbol_Team%20Project_v011.pdf)
- **Clock Speed:** 12MHz / **Sampling Frequency:** 600KHz
- **Input:** Signed 3-bit / **Output:** Signed 16-bit
- **가변 길이 대응:** 최대 63 ~ 최저 1 (홀수)
- **Folded Structure:** 32개의 계수 저장으로 최대 63-tap 구현.
- **Delay:** Group delay 31, Extra delay 2.

### 1.2. 주요 특징
- **Reconfigurable Coefficients SRAM:** 실시간 계수 업데이트 및 연산 모드 전환 가능.
- **병렬 MAC:** 4개의 SRAM과 MAC 유닛을 조합하여 병렬 구동.
- **Saturation Arithmetic:** 오버/언더 플로우 방지를 위한 클리핑 로직 적용.

---

## 2. 구현 설명

### 2.1. 최상위 모듈 (FirTop)
전체 필터 시스템의 최상위 모듈로, 제어부(Control Plane)와 데이터 처리부(Data Plane)를 통합합니다. 63-tap Delay Line(Shift Register)을 내장하고 있습니다.

### 2.2. 컨트롤 유닛
- **FIR_FSM_Controller:** 메인 FSM (IDLE, WRITE, MAC, SUM 상태 관리).
- **SeqMacController:** 대칭성을 고려하여 각 MAC 유닛의 Enable 신호를 동적으로 생성.
- **SRAM_Controller_4Parallel:** 상태에 따라 4개의 SRAM 뱅크 접근 권한을 중재(Arbitration).

### 2.3. 실행 유닛
- **SeqMac:** 곱셈 및 누적 연산 수행. 비트 폭이 확장된 ACC를 사용 후 saturation 처리.
- **saturation_fixer / AdderWithSat:** 데이터 무결성을 보장하기 위한 클리핑 로직.
- **SpSram:** 계수를 저장하는 16 Depth, 32 Bit 동기식 메모리 뱅크.

---

## 3. 시스템 흐름
1. **필터 연산:** 입력 데이터 저장 → FSM 기반 MAC 연산 시작 → SRAM 계수 병렬 인출 → 병렬 연산 및 합산 → 결과 출력.
2. **필터 업데이트:** 업데이트 플래그 설정 → SRAM 주소 중재 및 계수 저장 → IDLE 상태 복귀.

---

