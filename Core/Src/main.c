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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t Color_Frequency_Count = 0;
uint16_t current_pulse = 150;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 1. 在文件开头（main函数外面）定义一个全局变量，记录当前脉宽
void Servo_Move_To(uint16_t target_angle)
{
    // 计算目标角度对应的目标脉宽
    uint16_t target_pulse = (uint16_t)(50 + (target_angle * 200 / 180));

    // 自动判断：当前脉宽还没达到目标脉宽时，进入循环
    while (current_pulse != target_pulse)
    {
        if (current_pulse < target_pulse) {
            current_pulse++; // 当前位置比目标小，就递增
        } else {
            current_pulse--; // 当前位置比目标大，就递减
        }

        // 直接向定时器寄存器写入最新的微步脉宽
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, current_pulse);

        // 控速延迟（1~5ms）
        HAL_Delay(1);
    }
}
/* ================= 电机控制核心动作函数 ================= */

// 🚂 动作一：让传送带开始向前全速运转（启动）
void Conveyor_Start(void)
{
    // 让 PA0 喷出 3.3V 高电平，发送给 L298N 的 IN1 [CSDN, 2024-03-24]
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    // 让 PA1 保持 0V 低电平，发送给 L298N 的 IN2 [CSDN, 2024-03-24]
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

// 🛑 动作二：让传送带瞬间急停熄火（停止）
void Conveyor_Stop(void)
{
    // 将 PA0 和 PA1 同时拉低到 0V 绝对低电平 [CSDN, 2024-03-24]
    // L298N 内部的大闸门瞬间关闭，切断电池流入电机的电流，实现秒级急停！
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

// 🔄 动作三：让传送带往相反方向倒退（反转 - 留作调试应急或卡料排出备用）
void Conveyor_Reverse(void)
{
    // 把方向信号反过来：PA0 送低电平，PA1 送高电平 [CSDN, 2024-03-24]
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
  HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint32_t Red_Val=0;
      uint32_t Green_Val=0;

      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
         {
             HAL_Delay(20); // 软件消抖

             // 2. 再次确认按键是否真的按下
             if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
             {
                 // 3. 翻转 PC13 的电平（亮变灭，灭变亮）
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                //切换到红色通道，数100ms的脉冲数
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);//S2=0
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);//S3=0
                Color_Frequency_Count=0;
                HAL_Delay(100);
                Red_Val=Color_Frequency_Count;

                //切换到绿色通道，数100ms的脉冲数
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);//S2=1
                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//S3=1
                Color_Frequency_Count=0;
                HAL_Delay(100);
                Green_Val=Color_Frequency_Count;

                //颜色判断
                if(Red_Val > Green_Val+5000)
                {
                    Servo_Move_To(120);
                    HAL_Delay(2000);//等待物品传送
                    Servo_Move_To(60);
                    HAL_Delay(1000);
                    Servo_Move_To(90);
                }
                else if(Green_Val > Red_Val+1000)
                {
                    Servo_Move_To(60);
                    HAL_Delay(2000);//等待物品传送
                    Servo_Move_To(120);
                    HAL_Delay(1000);
                    Servo_Move_To(90);
                }
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
             }
             else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
         }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
