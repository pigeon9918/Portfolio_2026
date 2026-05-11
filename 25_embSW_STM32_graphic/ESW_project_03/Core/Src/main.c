/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "xpt2046.h"
#include "ili9341.h"
#include "render.h"
#include "button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// touch point struct
typedef struct  {
  uint16_t x;
  uint16_t y;
} Point;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG_MODE false
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

// global states for line buffer render & send
volatile int draw_ready = 1;
volatile int send_ready = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if DEBUG_MODE
int _write(int file, char * p, int len) {
  if (HAL_UART_Transmit(&huart1, (uint8_t *)p, len, 100) == HAL_OK)
    return len;
  return -1;
}
#endif


// volatile int touch_flag = 0;
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//   if( GPIO_Pin == XPT_IRQ_Pin ) {
//     touch_flag = 1;
//   }
// }

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI1) {
    draw_ready = 1;
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  
  /* USER CODE BEGIN 2 */
  #if DEBUG_MODE
  printf("getting stated...\r\n");
  #endif

  // touch variables
  point touch;
  Point last_touch_point = {0, 0};
  xpt_init(&touch, &hspi2, XPT_CS_GPIO_Port, XPT_CS_Pin);
  
  unsigned int last_touch_time = 0;
  unsigned int last_buffer_tick = 0;
  
  int is_touched = 0;
  int is_button_processed = 0;
  
  // renderer variables
  uint16_t arr[ILI9341_WIDTH * 2] ={};
  uint16_t* line_buf[2] = {arr, (arr + ILI9341_WIDTH)};
  unsigned int draw_num = 0;
  unsigned int line_num = 0;
  
  // lcd init
  ILI9341_Init();
  
  HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, 0);
  HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, 0);
  ILI9341_SetAddressWindow(0, 0, ILI9341_WIDTH - 1, ILI9341_HEIGHT - 1);
  HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, 1);
  
  RENDER_InitUI();
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // touch poller
    if(HAL_GPIO_ReadPin(XPT_IRQ_GPIO_Port, XPT_IRQ_Pin) == 0 && HAL_GetTick() - last_touch_time >= 10 && xpt_read(&touch)) {
      last_touch_time = HAL_GetTick(); // Update last touch time
      last_touch_point.x = touch.m_x;
      last_touch_point.y = touch.m_y;
      is_touched = 1;

      #if DEBUG_MODE
      printf("x: %d, y: %d\r\n", last_touch_point.x, last_touch_point.y);
      #endif
    }
    // touch release detector
    else if(HAL_GetTick() - last_touch_time >= 50){
      // touch_flag = 0;
      is_touched = 0;
      is_button_processed = 0;
    }
    
    // cursor noise reducer
    if(is_touched && (abs(last_touch_point.x - g_entities[0].x) > 5 || abs(last_touch_point.y - g_entities[0].y) > 5)) {
      // update cursor
      g_entities[0].x = touch.m_x;
      g_entities[0].y = touch.m_y;
    }
    
    // button hanlder
    if(is_touched && !is_button_processed) {
      is_button_processed = 1;

      // button hanlder 0
      if(BUTTON_POWER_X < touch.m_x && touch.m_x < BUTTON_POWER_X + BUTTON_WIDTH && BUTTON_POWER_Y < touch.m_y && touch.m_y < BUTTON_POWER_Y + BUTTON_WIDTH) {
        HAL_GPIO_TogglePin(INTERNAL_LED_GPIO_Port, INTERNAL_LED_Pin);

        #if DEBUG_MODE
        printf("power button\r\n");
        #endif
      }
      // button hanlder 1
      if(BUTTON_COLOR_X < touch.m_x && touch.m_x < BUTTON_COLOR_X + BUTTON_WIDTH && BUTTON_COLOR_Y < touch.m_y && touch.m_y < BUTTON_COLOR_Y + BUTTON_WIDTH) {
        change_bg_color();

        #if DEBUG_MODE
        printf("color button\r\n");
        #endif
      }
    }

    // draw line buffer
    if(draw_ready) {
      draw_ready = 0;

      draw_line(line_num, line_buf[draw_num]);
      send_ready = 1;

      if (++line_num == ILI9341_HEIGHT) {
        line_num = 0;
        end_of_frame();
      }
    }
    
    // send line buffer
    if(send_ready && HAL_GetTick() - last_buffer_tick >= 2 && HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_BUSY_TX) {
      send_ready = 0;

      HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, 0);
      HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, 1);
      // HAL_SPI_Transmit 함수를 사용하여 line_buf를 SPI로 보냄.
      HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)line_buf[draw_num], ILI9341_WIDTH*2);
      
      #if DEBUG_MODE
      printf("start sending line_buf[%d]\r\n", draw_num);
      #endif

      draw_num++;
      draw_num &= 1;
    }

    #if DEBUG_MODE
    HAL_Delay(500);
    #endif

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(INTERNAL_LED_GPIO_Port, INTERNAL_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ILI9341_DC_Pin|ILI9341_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(XPT_CS_GPIO_Port, XPT_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : INTERNAL_LED_Pin */
  GPIO_InitStruct.Pin = INTERNAL_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(INTERNAL_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ILI9341_DC_Pin ILI9341_CS_Pin */
  GPIO_InitStruct.Pin = ILI9341_DC_Pin|ILI9341_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : XPT_CS_Pin */
  GPIO_InitStruct.Pin = XPT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(XPT_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : XPT_IRQ_Pin */
  GPIO_InitStruct.Pin = XPT_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(XPT_IRQ_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#if USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    //  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
