/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdbool.h>
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
uint8_t rx2_data = 0;

uint16_t speed = 6000;
uint16_t speed_LR = 9000;
uint16_t speed_turn = 4000;

uint16_t speed_user = 2000;

uint32_t desired_speed = 3000;
uint32_t MOTER_PWM[4] = {0};
uint32_t Kp = 1;
uint32_t encoder_cnt[4] = {0};
uint32_t encoder_speed[4] = {0};
int32_t error_speed[4] = {0};
int32_t PID_speed[4] = {0};
uint32_t old_PID_speed[4] = {3000, 3000, 3000, 3000};

/***********lidar************/
//response data
bool scan_start = false;
uint8_t rx3_start[7] = {0};
uint8_t rx3_data[5] = {0};
uint8_t Q = 0;
bool S = false;
uint16_t angle;
uint16_t d;
uint16_t distance[360] = {0};
int16_t avg_DIFF = 0;

//protocal
uint8_t scan_command[2] = {0xA5,0x20};
uint8_t stop_command[2] = {0xA5,0x25};
uint8_t soft_reboot[2] = {0xA5,0x40};
uint8_t scan_express[2] = {0xA5,0x82};
uint8_t scan_force[2] = {0xA5,0x21};
uint8_t device_info[2] = {0xA5,0x50};
uint8_t health_status[2] = {0xA5,0x52};
uint8_t sample_rate[2] = {0xA5,0x59};
uint8_t scan_response[7] = {0xa5, 0x5a, 0x5, 0x0, 0x0, 0x40, 0x81};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool array_element_of_index_equal(uint8_t a[], uint8_t b[], uint8_t size) {
   uint8_t i;
   for(i=0; i<size; i++){
      if( a[i] != b[i] )
         return false;
   }
   return true;
}
int16_t array_avg_compare(uint16_t distance[]){

   uint32_t sum_R = 0;
   uint32_t sum_L = 0;
   uint8_t len_L = 0;
   uint8_t len_R = 0;
   uint16_t avg_R = 0;
   uint16_t avg_L = 0;
   int16_t avg_diff = 0;

   for(int i=0; i<90; i++){
      sum_R += distance[i];
      if(distance[i]!=0){
         len_R++;
      }
   }
   avg_R = sum_R/len_R;

   for(int i=270; i<360; i++){
      sum_L += distance[i];
      if(distance[i]!=0){
         len_L++;
      }
   }
   avg_L = sum_L/len_L;

   avg_diff = avg_R - avg_L; //왼쪽 거리가 클때 -

   return avg_diff;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)	//Timer interrupt every 20ms
{
	/*if(htim->Instance == TIM6){

		//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
		encoder_cnt[0] = TIM2->CNT;
		TIM2->CNT=0;

		encoder_cnt[1] = TIM3->CNT;
		TIM3->CNT=0;

		encoder_cnt[2] = TIM4->CNT;
		TIM4->CNT=0;

		encoder_cnt[3] = TIM5->CNT;
		TIM5->CNT=0;

		encoder_speed[0] = 164.18 * exp(0.0112*encoder_cnt[0]);
		encoder_speed[1] = 164.18 * exp(0.0112*encoder_cnt[1]);
		encoder_speed[2] = 164.18 * exp(0.0112*encoder_cnt[2]);
		encoder_speed[3] = 164.18 * exp(0.0112*encoder_cnt[3]);

		error_speed[0] = desired_speed - encoder_speed[0];
		error_speed[1] = desired_speed - encoder_speed[1];
		error_speed[2] = desired_speed - encoder_speed[2];
		error_speed[3] = desired_speed - encoder_speed[3];

		PID_speed[0] = old_PID_speed[0] + Kp*error_speed[0];
		PID_speed[1] = old_PID_speed[1] + Kp*error_speed[1];
		PID_speed[2] = old_PID_speed[2] + Kp*error_speed[2];
		PID_speed[3] = old_PID_speed[3] + Kp*error_speed[3];

		old_PID_speed[0] = PID_speed[0];
		old_PID_speed[1] = PID_speed[1];
		old_PID_speed[2] = PID_speed[2];
		old_PID_speed[3] = PID_speed[3];
	}
	*/
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM5_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(3000);

  //Initialize for motor PWM
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

  //Initialize for motor direction
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, RESET);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, RESET);

  //Initialize for encoder count value
  TIM2->CNT = 0;
  TIM3->CNT = 0;
  TIM4->CNT = 0;
  TIM5->CNT = 0;

  //Initialize for Encoder
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);

  //Initialize for timer interrupt initialization for Encoder (50ms)
  HAL_TIM_Base_Start_IT(&htim6);

  //LIDAR_scan_start
  //HAL_UART_Transmit(&huart3, &scan_command, 2, 100);
  HAL_UART_Transmit(&huart3, &scan_command, 2, 100);
  //robotArm to Cart
  HAL_UART_Receive_IT(&huart2,&rx2_data,1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, RESET);

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, RESET);
  while (1)
  {
	  	if(rx2_data == 1){				// 전방 카트와의 거리가 70cm 미만일 때 -> 정지
	  		TIM1->CCR1 = 0;
	  		TIM1->CCR2 = 0;
	  		TIM1->CCR3 = 0;
	  		TIM1->CCR4 = 0;

	  	} else if(rx2_data == 2){		// 전방 카트와의 거리가 70cm 이상 100cm 미만일 때 -> speed = 2400기준 16%씩 감소
	  		if (speed_user > 400){
	  			speed_user -= 400;
	  		}
	  		else{
	  			speed_user = 0
	  		}
	  		TIM1->CCR1 = speed_user;
	  		TIM1->CCR2 = speed_user;
	  		TIM1->CCR3 = speed_user;
	  		TIM1->CCR4 = speed_user;
	  		HAL_Delay(500);
	  	} else if(rx2_data == 3){		// 전방 카트와의 거리가 100cm 이상 150cm 미만일 때 -> speed = 2400기준 8%씩 감소
	  		if (speed_user > 200){
	  			speed_user -= 200;
	  		}
	  		TIM1->CCR1 = speed_user;
	  		TIM1->CCR2 = speed_user;
	  		TIM1->CCR3 = speed_user;
	  		TIM1->CCR4 = speed_user;
	  		HAL_Delay(500);
	  	} else if(rx2_data == 4){		// 전방에 카트가 없거나 거리가 150cm 이상일 때 -> speed = 2400기준 33%씩 증가
	  		if (speed_user < 1600) {
	  			speed_user = 2400;
	  		}
	  		else{
	  			speed_user += 800;
	  		}
	  		TIM1->CCR1 = speed_user;
	  		TIM1->CCR2 = speed_user;
	  		TIM1->CCR3 = speed_user;
	  		TIM1->CCR4 = speed_user;
	  		HAL_Delay(500);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* TIM6_DAC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART2){
     HAL_UART_Receive_IT(&huart2,&rx2_data,1);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
