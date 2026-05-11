# 고성능 AXI4 기반 3x3 이미지 Convolution 가속기 설계

**[2026년 졸업작품 - 선행 과제] Zynq-7000 SoC 기반 HW/SW Co-design**

### 발표자료
- **발표자료:** [PDF](./AXI_CONV.pdf)

## 1. 프로젝트 개요

Zynq-7000 SoC 플랫폼에서 동작하는 고성능 3x3 Convolution 연산 가속기 IP를 설계하고, 실제 하드웨어에서 성능을 검증한 프로젝트입니다. ARM 프로세서(PS)와 FPGA(PL) 간의 협력을 통해 순수 소프트웨어 처리 대비 **최대 44배의 성능 향상**을 달성했습니다.

-   **하드웨어 (RTL)**: Verilog HDL을 사용하여 4단 파이프라인 구조의 스트리밍 방식 Convolution Core를 설계했습니다.
-   **소프트웨어 (Driver)**: Bare-metal C 환경에서 AXI DMA 및 가속기 IP를 제어하는 드라이버를 작성하고, 하드웨어의 정합성을 검증했습니다.
-   **시스템 검증**: PYNQ (Python on Zynq) 프레임워크를 활용하여 실제 보드(PYNQ-Z2)에서 이미지 처리 성능을 측정하고 SW 처리(OpenCV)와 비교 분석했습니다.

## 2. 시스템 아키텍처

본 시스템은 Zynq의 ARM 프로세서(PS)와 FPGA(PL)가 AXI4 인터페이스를 통해 유기적으로 동작하도록 설계되었습니다.

-   **제어 경로 (Control Path)**: PS는 `AXI-Lite`를 통해 가속기 IP의 레지스터(동작 시작, 이미지 크기, 필터 계수)를 제어합니다.
-   **데이터 경로 (Data Path)**: `AXI DMA`가 DDR 메모리의 원본 이미지를 `AXI-Stream`을 통해 가속기 IP로 전달합니다. 가속기는 스트림 데이터를 실시간으로 처리하여 다시 `AXI-Stream`으로 DMA에 보내고, DMA는 최종 결과 이미지를 DDR 메모리에 저장합니다.

## 3. 핵심 성능 지표 (PYNQ-Z2, PL @ 125MHz)

`Jupyter_files/benchmark.ipynb`를 통해 측정한 결과, 다양한 해상도에서 OpenCV (`cv2.filter2D`) 대비 높은 성능 향상률을 확인했습니다.

| Resolution   | HW Time (s) | OpenCV Time (s) | **Speedup** | Throughput (MB/s) |
| :----------- | :---------- | :-------------- | :---------- | :---------------- |
| 640 x 480    | 0.00344     | 0.11440         | **33.23x**  | 352.5             |
| 1280 x 720   | 0.00847     | 0.34732         | **41.01x**  | 436.4             |
| 1920 x 1080  | 0.01779     | 0.78593         | **44.18x**  | 466.4             |

-   **최대 처리율 (Throughput)**: **466.4 MB/s** (FHD 해상도 기준, AXI-Stream to DDR 이론상 대역폭 500MB/s의 **93%** 달성)
-   **저지연성 (Low Latency)**: 최초 픽셀 출력까지 약 `7 + Image Width` 클럭 사이클이 소요됩니다. 이는 프레임 전체를 버퍼링하지 않고 실시간으로 처리하는 스트리밍 아키텍처의 장점을 극대화한 결과입니다.

## 4. 설계 상세

### 4.1. 하드웨어 (RTL) - `Verilog/HDL/`

-   **`Conv_core_RGB.v`**: Convolution 연산의 핵심 로직입니다.
    -   **스트리밍 아키텍처**: AXI-Stream 입력을 받아 픽셀 단위로 즉시 처리 후 출력하는 완전한 스트리밍 구조로 설계하여 BRAM과 같은 내부 버퍼 메모리 사용을 최소화했습니다.
    -   **4-Stage Pipeline**: `Window Shift` -> `MAC` -> `Sum` -> `Accumulate & Saturation`의 4단 파이프라인을 적용하여 클럭 사이클당 1픽셀을 처리(`CPI≈1`)할 수 있도록 최적화했습니다. DSP 슬라이스 사용을 명시(`use_dsp = "yes"`)하여 효율적인 연산을 유도했습니다.
    -   **Line Buffer 최적화**: 3x3 윈도우 구성에 필요한 이전 2라인의 픽셀 데이터를 저장하기 위해, **SPSRAM** 기반의 FIFO(`FIFO.v`, `SpSram.v`)를 3개 사용하여 로직 리소스를 절약했습니다.
    -   **Zero Padding**: 이미지 경계(edge) 처리를 위한 Zero Padding 로직을 FSM 없이 순수 조합논리로 구현하여 지연 시간을 줄였습니다.

-   **`convcore_AXIS_wrapper.v`**:
    -   순수 연산 코어(`Conv_core_RGB`)와 시스템 버스(AXI-Stream)를 연결하는 Wrapper입니다. 출력단에 `Queue_cir.v` (원형 큐)를 두어 데이터 경로의 후단에서 발생하는 back-pressure에 유연하게 대처할 수 있도록 설계했습니다.

### 4.2. 소프트웨어 (Driver & Verification) - `C/` 및 `Jupyter_files/`

-   **`C/filter.c`**: ARM Core에서 실행되는 Bare-metal C 코드입니다.
    -   **주요 기능**:
        1.  **가속기 IP 제어**: 이미지 크기, 필터 계수 등을 `AXI-Lite`를 통해 IP 레지스터에 기록합니다.
        2.  **정합성 검증**: 동일한 이미지를 C코드로 직접 연산한 결과(`SW_DST_ADDR`)와 가속기가 처리한 결과(`HW_DST_ADDR`)를 메모리상에서 직접 비교하여 하드웨어의 정확성을 100% 검증합니다.

-   **`Jupyter_files/*.ipynb`**: PYNQ(Python on Zynq) 환경에서 실제 하드웨어의 성능을 측정하고 시각화합니다. Python을 통해 DMA 버퍼를 할당하고 IP를 제어하며, `time` 라이브러리와 `OpenCV`를 이용해 정량적인 성능 비교 리포트를 생성합니다.

