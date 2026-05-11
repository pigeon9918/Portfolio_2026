/*
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdint.h>
#include <xil_types.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xaxidma.h" // AXI DMA 드라이버 헤더 추가
#include "xscugic.h" // GIC driver

/* --------------------------------------------------------------------------
 * 1. Custom IP Register Structure
 * -------------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t CONFIG;      // 0x00: [0] En, [4] Reset (Self-clearing)
    volatile uint32_t SIZE;        // 0x04: [15:0] Width, [31:16] Height
    volatile uint32_t STATUS;      // 0x08: [0] Done (Read Only)
    volatile uint32_t COEFF_R[3];  // 0x0C, 0x10, 0x14
    volatile uint32_t COEFF_G[3];  // 0x18, 0x1C, 0x20
    volatile uint32_t COEFF_B[3];  // 0x24, 0x28, 0x2C
    volatile uint32_t CNT;         // 0x30
    volatile uint32_t DEBUG_READY; // 0x34
} HP_CONV_REGS;

HP_CONV_REGS *my_hw_ip = (HP_CONV_REGS*)(XPAR_MYIP_0_BASEADDR);

// 메모리 주소 정의 (DDR 메모리 중 사용되지 않는 영역 지정)
#define IMG_WIDTH   1920
#define IMG_HEIGHT  1080
#define PIXEL_SIZE  4  // uint32_t (RGBA 등 4바이트 기준)

// DATA 영역을 통해서 code에 방해되지 않는 DDR 영역 확보(linker script 참고)
uint32_t SRC_ADDR[IMG_HEIGHT][IMG_WIDTH] __attribute__ ((aligned (32)));

uint32_t SW_DST_ADDR[IMG_HEIGHT][IMG_WIDTH] __attribute__ ((aligned (32)));
uint32_t HW_DST_ADDR[IMG_HEIGHT][IMG_WIDTH] __attribute__ ((aligned (32)));

UINTPTR ReadAddr = (UINTPTR)SRC_ADDR;
UINTPTR WriteAddr = (UINTPTR)HW_DST_ADDR;

const int8_t kernel[3][3] = {
    {-1, -1, -1},
    {-1, 9, -1},
    {-1, -1, -1}
};


// DMA 객체 전역 선언
XAxiDma AxiDma;

int main() {
    init_platform();

    printf("Hello world!\n\r");
    printf("Starting convolution IP test!\n\r");

    printf("kernel) \n\r %d, %d, %d\n\r%d, %d, %d\n\r%d, %d, %d\n\r"
        , kernel[0][0], kernel[0][1], kernel[0][2]
        , kernel[1][0], kernel[1][1], kernel[1][2]
        , kernel[2][0], kernel[2][1], kernel[2][2]);

    printf("Width: %u, Height: %u\n\r", IMG_WIDTH, IMG_HEIGHT);
        
    printf("SRC: %x, DST: %x\n\r", ReadAddr, WriteAddr);

    // 1. DMA 초기화
    XAxiDma_Config *CfgPtr;

    xil_printf("DMA Hardware is healthy.\r\n");


    // xparameters.h에 정의된 DMA Device ID 사용 (Vivado 버전에 따라 이름이 다를 수 있음)
    CfgPtr = XAxiDma_LookupConfig(XPAR_XAXIDMA_0_BASEADDR); 
    if (!CfgPtr) {
        printf("DMA Configuration failed!\n\r");
        return -1;
    }

    int Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
    if (Status != XST_SUCCESS) {
        printf("DMA Initialization failed!\n\r");
        return -1;
    }

    // 2. Selftest 수행
    Status = XAxiDma_Selftest(&AxiDma);
    if (Status != XST_SUCCESS) {
        xil_printf("Error: DMA Hardware is not responding!\r\n");
        return XST_FAILURE;
    }

    // 인터럽트를 사용하지 않고 Polling 방식을 사용할 것이므로 Disable 처리
    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

    // printf("please upload image and continue!\n\r");
    // printf("\n\r");

    // 4. 소스 데이터 준비 (CPU가 직접 쓰기)    
    for(uint32_t i=0; i < IMG_HEIGHT; i++) {
        for(uint32_t j=0; j < IMG_WIDTH; j++) {
            // {8'(y+1), 8'(x+1), 8'(y+x+1), 8'd0}
            SRC_ADDR[i][j] = (uint32_t)((j + 1) << 24) | ((i + 1) << 16) | ((i + j + 1) << 8);
            HW_DST_ADDR[i][j] = 0;
        }
    }

    // --- Generate Expected Image (3x3 Conv + Saturation) ---
    for (int y=0; y<IMG_HEIGHT; y=y+1) {
        for (int x=0; x<IMG_WIDTH; x=x+1) {
            int sum_ch2 = 0;
            int sum_ch1 = 0;
            int sum_ch0 = 0;

            for (int ky=-1; ky<=1; ky=ky+1) {
                for (int kx=-1; kx<=1; kx=kx+1) {
                    int py = y + ky;
                    int px = x + kx;

                    // Zero-Padding Boundary Check
                    if (py >= 0 && py < IMG_HEIGHT && px >= 0 && px < IMG_WIDTH) {
                        // 각 픽셀 채널 추출 (32bit: {ch2, ch1, ch0, 8'd0})
                        int p_ch2 = (SRC_ADDR[py][px] >> 16) & 0xFF;
                        int p_ch1 = (SRC_ADDR[py][px] >> 8 ) & 0xFF;
                        int p_ch0 = (SRC_ADDR[py][px] >> 0 ) & 0xFF;

                        // MAC 연산
                        sum_ch2 = sum_ch2 + (kernel[ky+1][kx+1] * p_ch2);
                        sum_ch1 = sum_ch1 + (kernel[ky+1][kx+1] * p_ch1);
                        sum_ch0 = sum_ch0 + (kernel[ky+1][kx+1] * p_ch0);
                    }
                }
            }

            // Saturation: 0 미만은 0, 255 초과는 255
            int val_ch2 = (sum_ch2 < 0) ? 0 : (sum_ch2 > 255) ? 255 : sum_ch2;
            int val_ch1 = (sum_ch1 < 0) ? 0 : (sum_ch1 > 255) ? 255 : sum_ch1;
            int val_ch0 = (sum_ch0 < 0) ? 0 : (sum_ch0 > 255) ? 255 : sum_ch0;

            // 정답 배열에 저장
            SW_DST_ADDR[y][x] = (val_ch2 << 16) | (val_ch1 << 8) | (val_ch0 << 0) | (0xFF << 24);
        }
    }

    // CPU가 캐시에만 쓰고 아직 RAM에 안 내렸을 수 있으므로 Flush (매우 중요)
    Xil_DCacheFlushRange((UINTPTR)SRC_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);
    Xil_DCacheFlushRange((UINTPTR)HW_DST_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);
    
    // (선택) Flush 직후 Invalidate는 굳이 안 해도 되지만 놔두겠습니다.
    Xil_DCacheInvalidateRange((UINTPTR)SRC_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);
    Xil_DCacheInvalidateRange((UINTPTR)HW_DST_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);

    // // 데이터 쓰기 정상 확인
    // for(uint32_t i=0; i < IMG_HEIGHT; i++) {
    //     for(uint32_t j=0; j < IMG_WIDTH; j++) {
    //         if(SRC_ADDR[i][j] != (uint32_t)(((j + 1) << 24) | ((i + 1) << 16) | ((i + j + 1) << 8))) {
    //             printf("Error writing at source!\n\r");
    //             break;
    //         }
    //     }
    // }

    // for(int loop=0; loop<5; loop++) {

    // 6. 커스텀 가속기 IP 시작
    my_hw_ip->CONFIG = (1 << 4); // Reset
    my_hw_ip->SIZE = ((uint16_t)(IMG_HEIGHT - 1) << 16) | (uint16_t)(IMG_WIDTH - 1);

    // 부호 확장(Sign Extension) 방지를 위한 0xFF 마스킹 적용
    my_hw_ip->COEFF_R[0] = ((kernel[0][0] & 0xFF) << 16) | ((kernel[0][1] & 0xFF) << 8) | (kernel[0][2] & 0xFF);
    my_hw_ip->COEFF_R[1] = ((kernel[1][0] & 0xFF) << 16) | ((kernel[1][1] & 0xFF) << 8) | (kernel[1][2] & 0xFF);
    my_hw_ip->COEFF_R[2] = ((kernel[2][0] & 0xFF) << 16) | ((kernel[2][1] & 0xFF) << 8) | (kernel[2][2] & 0xFF);
    
    my_hw_ip->COEFF_G[0] = ((kernel[0][0] & 0xFF) << 16) | ((kernel[0][1] & 0xFF) << 8) | (kernel[0][2] & 0xFF);
    my_hw_ip->COEFF_G[1] = ((kernel[1][0] & 0xFF) << 16) | ((kernel[1][1] & 0xFF) << 8) | (kernel[1][2] & 0xFF);
    my_hw_ip->COEFF_G[2] = ((kernel[2][0] & 0xFF) << 16) | ((kernel[2][1] & 0xFF) << 8) | (kernel[2][2] & 0xFF);
    
    my_hw_ip->COEFF_B[0] = ((kernel[0][0] & 0xFF) << 16) | ((kernel[0][1] & 0xFF) << 8) | (kernel[0][2] & 0xFF);
    my_hw_ip->COEFF_B[1] = ((kernel[1][0] & 0xFF) << 16) | ((kernel[1][1] & 0xFF) << 8) | (kernel[1][2] & 0xFF);
    my_hw_ip->COEFF_B[2] = ((kernel[2][0] & 0xFF) << 16) | ((kernel[2][1] & 0xFF) << 8) | (kernel[2][2] & 0xFF);

    my_hw_ip->CONFIG = (1 << 0); // Enable

    // 7. DMA 시작
    uint32_t total_bytes = IMG_WIDTH * IMG_HEIGHT * PIXEL_SIZE;

    // [핵심] 가속기가 데이터를 뱉기 시작할 때 놓치지 않도록, 수신(S2MM) 채널을 먼저 대기시킵니다.
    int status_s2mm = XAxiDma_SimpleTransfer(&AxiDma, WriteAddr, total_bytes, XAXIDMA_DEVICE_TO_DMA);

    // printf("DMA_m DEBUG_READY Status: %x\n\r", my_hw_ip->DEBUG_READY);
    // while(my_hw_ip->DEBUG_READY == 0) {
    //     printf("\r.  ");
    //     usleep(50000);
    //     printf(".");
    //     usleep(50000);
    //     printf(".");
    //     usleep(50000);
    // }

    // 수신 준비가 끝났으므로, 송신(MM2S) 채널을 켜서 데이터를 가속기로 밀어넣습니다.
    int status_mm2s = XAxiDma_SimpleTransfer(&AxiDma, ReadAddr, total_bytes, XAXIDMA_DMA_TO_DEVICE);
    
    printf("MM2S Start: %d, S2MM Start: %d\n\r", status_mm2s, status_s2mm);
    printf("Setting Done! Waiting for DMA...\n\r");
    
    uint32_t timeout = 1000000; // 적절한 대기 시간 설정
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
        timeout--;
        if (timeout == 0) {
            printf("DMA Timeout Occurred!\n\r");
            // Timeout 발생 직후에 추가하여 상태 확인
            uint32_t s2mm_sr = XAxiDma_ReadReg(AxiDma.RegBase + XAXIDMA_RX_OFFSET, XAXIDMA_SR_OFFSET);
            uint32_t mm2s_sr = XAxiDma_ReadReg(AxiDma.RegBase + XAXIDMA_TX_OFFSET, XAXIDMA_SR_OFFSET);
            
            printf("Filter Counter Status: %x\n\r", my_hw_ip->CNT);
            printf("[DEBUG] S2MM Status: 0x%08x\n\r", s2mm_sr);
            printf("[DEBUG] MM2S Status: 0x%08x\n\r", mm2s_sr);
            break;
        }
    }
    timeout = 1000000;
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) {
        timeout--;
        usleep(1);
        if (timeout == 0) {
            printf("DMA Timeout Occurred!\n\r");
            // Timeout 발생 직후에 추가하여 상태 확인
            uint32_t s2mm_sr = XAxiDma_ReadReg(AxiDma.RegBase + XAXIDMA_RX_OFFSET, XAXIDMA_SR_OFFSET);
            uint32_t mm2s_sr = XAxiDma_ReadReg(AxiDma.RegBase + XAXIDMA_TX_OFFSET, XAXIDMA_SR_OFFSET);
            
            printf("Filter Counter Status: %x\n\r", my_hw_ip->CNT);
            printf("[DEBUG] S2MM Status: 0x%08x\n\r", s2mm_sr);
            printf("[DEBUG] MM2S Status: 0x%08x\n\r", mm2s_sr);
            break;
        }
    }


    printf("DMA Transfer Complete!\n\r");

    // 가속기가 써놓은 RAM 데이터를 CPU 캐시로 가져오기 위해 Invalidate 처리
    Xil_DCacheInvalidateRange((UINTPTR)SRC_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);
    Xil_DCacheInvalidateRange((UINTPTR)HW_DST_ADDR, IMG_HEIGHT * IMG_WIDTH * PIXEL_SIZE);

    uint32_t cnt = 0;
    for(uint32_t i=0; i < IMG_HEIGHT; i++) {
        for(uint32_t j=0; j < IMG_WIDTH; j++) {
            if (HW_DST_ADDR[i][j] != SW_DST_ADDR[i][j]) {
                printf("Error at [%02u][%02u], HW_DST_ADDR : %08x, SW_DST_ADDR : %08x\n\r", 
                        i, j, (unsigned int)HW_DST_ADDR[i][j], (unsigned int)SW_DST_ADDR[i][j]);
                cnt++;
            }
            // else {
            //     printf("[%02u][%02u], HW_DST_ADDR : %08x, SW_DST_ADDR : %08x\n\r", 
            //             i, j, (unsigned int)HW_DST_ADDR[i][j], (unsigned int)SW_DST_ADDR[i][j]);
            // }
        }
    }

    
    if(cnt != 0)
        printf("Error occured! Total mismatched pixels: %d\n\r", cnt);
    else
        printf("Completed without error!\n\r");

    // }

    cleanup_platform();
    
    return 0;
}