# [Image Processing on ARM Core] ARM 어셈블리를 이용한 이미지 처리 및 최적화
### 발표자료
- **보고서:** [PDF](./Report_7팀.pdf)
- **발표자료:** [PDF](./Presentation_7팀.pdf)

## Ⅰ. 프로젝트 개요

### Ⅰ-1. 프로젝트 주제
본 프로젝트는 32-bit RGBA 형식의 이미지를 SW를 이용하여 다양한 방식으로 변환하고, ARM 어셈블리 언어 및 저수준 기법을 이용하여 성능 향상을 도모하는 것을 목표로 합니다. 

**주요 이미지 처리 작업 (서브 프로젝트):**
- **Red Pixel Count:** 이미지에서 Red 값이 128 이상인 픽셀의 개수를 계산하는 함수 구현
- **RGB Negative 변환:** RGB 값을 각각 255에서 뺀 색상 반전 이미지를 생성하는 함수 구현
- **Grayscale 변환:** RGB 값을 가중합($3R + 6G + B$)을 이용하여 16-bit Grayscale 이미지로 변환하는 함수 구현

### Ⅰ-2. 진행 방향
1. **기능 검증 (C 언어):** 각 변환 함수를 C 언어로 구현하여 정확성을 검증하고, Keil uVision 시뮬레이션 환경을 통해 함수 수행 전후의 메모리 상태를 확인하였습니다.
2. **성능 최적화 (ARM Assembly):** Performance Analyzer를 통한 분석 후, 성능 최적화를 위해 inline assembly 함수를 구현하였습니다.
3. **Memory Relocation 도입:** 기존 32-bit RGBA 포맷에서 Alpha 채널을 제외하고 R, G, B 채널을 각각 연속된 메모리 공간에 분리 저장하는 **Planar 메모리 구조**를 구성하였습니다. 이를 통해 연산 시 불필요한 메모리 접근을 줄이고 캐시와 BUS 활용도를 높였습니다.
4. Shift연산, bit flip 등 저수준 기법을 통해서 연산을 최적화 하였습니다.
5. **결과 분석:** 최적화 전후의 함수 실행 시간을 분석하였으며,최종적으로 C 기반 결과와 ARM 최적화 버전의 출력 일치 여부를 검증하여 정확성과 성능 향상을 종합적으로 평가하였습니다.

---

## Ⅱ. 시스템 구성 및 파일 구조

### Ⅱ-1. 주요 소스 코드 구성
- **메인 로직:**
  - `main.c`: 기본적인 C 및 ASM 함수 호출 및 검증 로직.
  - `main_reallo.c`: Memory Relocation(Planar 구조)이 적용된 최적화 버전의 실행 로직.
- **이미지 처리 함수 (C Implementation):**
  - `convertGray.c`, `reAllocation.c`: 이미지 변환 및 재배치 로직의 C 언어 구현체.
  - `find_IDAT.c`: PNG 이미지 데이터 섹션(IDAT)의 시작점을 찾는 유틸리티.
  - `isSame.c`: C 언어 결과와 ASM 결과의 일치 여부를 확인하는 검증 함수.
- **최적화 함수 (ARM Assembly):**
  - `countRed.s`, `convertReverse.s`, `convertGray.s`: 기본적인 어셈블리 구현.
  - `countRedRel.s`, `convertReverseRel.s`, `convertGrayRel.s`: Planar 구조 및 블록 전송 기법이 적용된 **고도로 최적화된 어셈블리 구현**.

---

## Ⅲ. 주요 알고리즘 및 최적화 기법

### Ⅲ-1. 이미지 변환 알고리즘
1. **Grayscale ($3R + 6G + B$):**
   - 단순 평균이 아닌 가중합을 사용하여 인간의 시각적 인지 특성을 반영.
   - ASM 구현 시 `LSL` (Logical Shift Left) 명령어를 활용하여 곱셈 연산을 덧셈과 시프트 연산으로 대체함으로써 속도를 향상시켰습니다.
2. **Negative (반전):**
   - 각 채널의 값을 `255 - value`로 변환.
   - ASM에서는 `MVN` (Move Not) 또는 `RSB` (Reverse Subtract) 명령어를 사용하여 최적화하였습니다.

### Ⅲ-2. 고수준 최적화 기법 (Relocation & Block Processing)
최적화된 버전(`*Rel.s`)에서는 다음과 같은 기법을 적용하여 성능을 극대화하였습니다.

1. **Memory Relocation (Planar Data Structure):**
   - 기존 인터리브(Interleaved) 방식(R-G-B-A-R-G-B-A)에서 채널별 분리(planar) 방식(RRRR... GGGG... BBBB...)으로 메모리 구조를 변경.
   - 불필요한 Alpha 채널 접근을 제거하고, 동일 채널 데이터에 대한 연속 접근성을 확보하여 메모리 대역폭 효율을 높였습니다.

2. **Block Data Transfer (`LDMIA` / `STMIA`):**
   - 한 번의 메모리 접근으로 여러 개의 픽셀 데이터를 레지스터로 로드.
   - `LDMIA r0!, {r5-r12}`와 같이 다중 레지스터 로드 명령어를 사용하여 메모리 접근 횟수를 획기적으로 줄였습니다.

3. **Loop Unrolling & Logic Optimization:**
   - 반복문 내부에서 처리하는 데이터 양을 늘려 루프 제어 오버헤드를 감소시켰습니다.
   - `Red Pixel Count` 최적화 시, `TST` 명령어와 `ADDNE` 등의 조건부 실행 명령어를 조합하여 분기(Branch) 발생을 최소화하였습니다.

---

## Ⅳ. 성능 평가 및 결론
Keil uVision의 **Performance Analyzer**를 통해 분석한 결과, C 언어 대비 단순 ASM 전환 시에도 유의미한 성능 향상이 있었으며, 특히 **Memory Relocation과 Block Processing을 적용한 최종 최적화 버전**에서는 명령어 개수(Instruction Count)와 실행 사이클이 대폭 감소하는 결과를 얻었습니다. 이는 임베디드 시스템에서 하드웨어 구조에 최적화된 저수준 프로그래밍의 중요성을 입증합니다.
