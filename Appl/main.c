/**
  ******************************************************************************
  * @file    Project/STM8L15x_StdPeriph_Template/main.c
  * @author  MCD Application Team
  * @version V1.6.1
  * @date    30-September-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "led.h"
#include "cp_comm.h"
#include "box_open.h"
    
const u8 ver_FMW[4] = "0019";


#define F_CPU           CLK_GetClockFreq()
#define dly_const       (F_CPU / 37500000)

/** @addtogroup STM8L15x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



/* Private function prototypes -----------------------------------------------*/
void Counters_input_init(void);
void Timer_init(void);
/* Private functions ---------------------------------------------------------*/

u16 timeout_sec = 0;

void delay_us(uint32_t value)
{
  uint32_t i;
  for(i = value*dly_const; i > 0; i--)
  asm("nop");
  //IWDG->KR = 0xAA;
}

//void test_send_n_pulses(u8 cnt_pulses)
//{    
//    while(cnt_pulses--)
//    {
//        delay_us(1000);
//      LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_ON, 0,0,0);
//      delay_us(1000);
//      LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_OFF, 0,0,0);
//    }
//}

void delay_ms(unsigned int  value)
{
  while(value)
  {
    delay_us(1000);
    value--;
  };
}
     
void handle_counters(u16 *ptr_cnt1, u16 *ptr_cnt2)
{
    static u8 status_led1 = 0, status_led2 = 0;
    u16 new_cnt = TIM2_GetCounter();
    static u16 internal_cnt1 = 0;
    static u16 internal_cnt2 = 0;
    //TIM2_SetCounter(0); /* reset counter */
    
    /* check that cnt was changed */
    if(new_cnt != internal_cnt1)
    {
        internal_cnt1 = new_cnt;
        *ptr_cnt1 = internal_cnt1;
        status_led1 = !status_led1;
        
        if(status_led1)
            LED_set(PULSE_COUNTER_1_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
        else
            LED_set(PULSE_COUNTER_1_LED_GREEN, LED_MODE_ON, 0, 0, 50);
    }

    new_cnt = TIM3_GetCounter();
    //TIM3_SetCounter(0); /* reset counter */
    
    /* check that cnt was changed */
    if(new_cnt != internal_cnt2)
    {
        internal_cnt2 = new_cnt;
        *ptr_cnt2 = internal_cnt2;
        status_led2 = !status_led2;
        
        if(status_led2)
            LED_set(PULSE_COUNTER_2_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
        else
            LED_set(PULSE_COUNTER_2_LED_GREEN, LED_MODE_ON, 0, 0, 50);
    }         
}

void set_out_data(const u8 *ptr_data, u8 size_of_data)
{
    if(size_of_data < MAX_TX_BUFFER)
    {
        I2C_transmitted_packet.number_of_bytes_sent = size_of_data;
        while(size_of_data--)
        {
            tx_Buffer[size_of_data] = ptr_data[size_of_data];
        }
    }
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
    u16 get_cnt1 = 0, get_cnt2 = 0;
    u16 prev_cnt1 = 0, prev_cnt2 = 0;
    u8 get_ovr1 = 0, get_ovr2 = 0;
    u8 tx_byte = 0;
    LED_TypeDef led;
    LED_Mode led_mode;
    u16 duration_ON, duration_OFF, duration_total;
    
    /* Set High speed internal clock prescaler */
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1); /* main clk is 16MHz */

    LED_init();

    CP_COMM_Init();
  
    LED_all_off();
  
    Counters_input_init();    /* Timer2 and Timer3 used for capture */
        
    BOXSTS_Init();
  //  test_send_n_pulses(99);
    
    Timer_init(); /* init timer for handle seconds */
    enableInterrupts();
    
/* in case box opened turn ON RED system health */    
    if(read_status_box_open() == 1)
        LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_ON, 0, 0, 0);
    
    /* during boot system blink green sys health */
    LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_FLASH, 100, 100, 0);
    
  while (1)
  {
     /* Reload IWDG counter */
    IWDG_ReloadCounter();
    
    /* read counters */
    handle_counters(&get_cnt1, &get_cnt2);
    
    /* check overflow */
    if(get_cnt1 < prev_cnt1)
    {
        ++get_ovr1;
    }
    
    if(get_cnt2 < prev_cnt2)
    {
        ++get_ovr2;
    }
    
    prev_cnt1 = get_cnt1;
    prev_cnt2 = get_cnt2;
    
    /* check sys run */
    /* if above 5 sec that mean system boot (reboot) */
    if(timeout_sec > 5)
    {
        timeout_sec = 0;
        LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_FLASH, 100, 100, 0);
    }

       
    /* check incomimng message */
    if( I2C_received_packet.ready )
    {
      if(check_summ() == TRUE)
      {
        timeout_sec = 0;
         
        switch( I2C_received_packet.instruction )
        { 
          case GET_VER_APL:
            set_out_data(ver_FMW, 4);
          break;
          
          case DO_RESET: 
            tx_byte = (u8)ACK;
            set_out_data(&tx_byte, 1);
            
            while(1); /* infinite loop until WDT reset */
            
          break;
          
          case SET_LEDS:
            tx_byte = (u8)ACK;
            set_out_data(&tx_byte, 1);
            
            led = (LED_TypeDef)I2C_received_packet.ptr_data[0];
            led_mode = (LED_Mode)I2C_received_packet.ptr_data[1];
            duration_ON = ((u16)I2C_received_packet.ptr_data[2]<<8) | I2C_received_packet.ptr_data[3];
            duration_OFF = ((u16)I2C_received_packet.ptr_data[4]<<8) | I2C_received_packet.ptr_data[5];
            duration_total = ((u16)I2C_received_packet.ptr_data[6]<<8) | I2C_received_packet.ptr_data[7];
            
            LED_set(led, led_mode, duration_ON, duration_OFF, duration_total);
            
          break;
          
          case GET_COUNTER1:
            tx_Buffer[0] = get_ovr1;
            tx_Buffer[1] = (u8)(get_cnt1>>8);
            tx_Buffer[2] = (u8)(get_cnt1&0xFF);
            I2C_transmitted_packet.number_of_bytes_sent = 3;
          break;
          
          case GET_COUNTER2:
            tx_Buffer[0] = get_ovr2;
            tx_Buffer[1] = (u8)(get_cnt2>>8);
            tx_Buffer[2] = (u8)(get_cnt2&0xFF);
            I2C_transmitted_packet.number_of_bytes_sent = 3;
          break;
                  
           case SET_COUNTER1:
                get_ovr1 = 0;
                get_cnt1 = ((u16)I2C_received_packet.ptr_data[0]<<8) | I2C_received_packet.ptr_data[1];
                TIM2_SetCounter(get_cnt1); /* reset counter */
                tx_byte = (u8)ACK;
                set_out_data(&tx_byte, 1);
                prev_cnt1 = 0;
           break;
            
           case SET_COUNTER2:
                get_ovr2 = 0;
                get_cnt2 = ((u16)I2C_received_packet.ptr_data[0]<<8) | I2C_received_packet.ptr_data[1];
                TIM3_SetCounter(get_cnt2); /* reset counter */
                tx_byte = (u8)ACK;
                set_out_data(&tx_byte, 1);
                prev_cnt2 = 0;
           break;
            
           case GET_OPEN_BOX_STATUS:
                tx_byte = (u8)read_status_box_open();             
                set_out_data(&tx_byte, 1);
                BOXSTS_SetAlarm(0);
                LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_OFF, 0, 0, 0);
           break;
          
          default: 
            tx_byte = (u8)NACK;
            set_out_data(&tx_byte, 1);
            break;
        }
      }
      else
      {
          tx_byte = (u8)NACK;
          set_out_data(&tx_byte, 1);
      }
      I2C_received_packet.ready = FALSE;
    }
  }
}


void Counters_input_init(void)
{
    SYSCFG_REMAPPinConfig(REMAP_Pin_TIM2TRIGPortA, ENABLE); /* tmr2 remap to A4 */
    SYSCFG_REMAPPinConfig(REMAP_Pin_TIM3TRIGPortA, ENABLE); /* tmr2 remap to A5 */
    
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
    
    TIM2_DeInit();
    TIM3_DeInit();
    
    GPIO_Init(GPIOA, (GPIO_Pin_4|GPIO_Pin_5), GPIO_Mode_In_PU_No_IT); /* counters gpio */
    
    TIM2_TimeBaseInit(TIM2_Prescaler_1, TIM2_CounterMode_Up, 0xFFFF);
    TIM2_ETRClockMode2Config(TIM2_ExtTRGPSC_OFF, TIM2_ExtTRGPolarity_NonInverted, 0);
    TIM2_Cmd(ENABLE);
    
    TIM3_TimeBaseInit(TIM3_Prescaler_1, TIM3_CounterMode_Up, 0xFFFF);
    TIM3_ETRClockMode2Config(TIM3_ExtTRGPSC_OFF, TIM3_ExtTRGPolarity_NonInverted, 0);
    TIM3_Cmd(ENABLE);

}

/*
    init timer for second handle
*/
void Timer_init(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
    TIM4_DeInit();
    
    //timer freq = (clock CPU/16) -> 1bit = 1uS -> 10000*1uS=10 mS
    //overflow 0.52224 s
    TIM4_TimeBaseInit(TIM4_Prescaler_32768, 0xFF);
    TIM4_Cmd(ENABLE);
    
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
