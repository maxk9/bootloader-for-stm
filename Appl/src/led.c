// ****************************************************************************
//
//  led.c
//
//
// ****************************************************************************
/*
******************************************************************************
*                         INCLUDES
******************************************************************************
*/
#include "led.h"

//#include "cmsis_os.h"
//#include "timers.h"
//#include "task.h"

#include <string.h>

/*
******************************************************************************
*                         DEFINES
******************************************************************************
*/

//#define USE_SEFE_LED_INFC

#define TIMER_PERIOD                        10000
#define TIMER_REPETITION_COUNTER            0
#define TIMER_PRESCALER                     5

#define LED_TIMER_NAME                      "LED_Timer"
#define LED_TIMER_TICK                      10              // unit of msec
#define LED_TIMER_CREATE_BLOCK_TIMEOUT      1000            // unit of msec 
#define LED_TIMER_STOP_BLOCK_TIMEOUT        1000            // unit of msec

//#define POWER_STATUS_LED_GREEN_GPIO_Port                        GPIOD
//#define POWER_STATUS_LED_GREEN_GPIO_Pin                         GPIO_Pin_2
#define INTERNET_STATUS_LED_BLUE_GPIO_Port                      GPIOD
#define INTERNET_STATUS_LED_BLUE_GPIO_Pin                       GPIO_Pin_2
#define INTERNET_STATUS_LED_RED_GPIO_Port                       GPIOD
#define INTERNET_STATUS_LED_RED_GPIO_Pin                        GPIO_Pin_3
#define OVERFILL_ALARM_LED_GREEN_GPIO_Port                      GPIOB
#define OVERFILL_ALARM_LED_GREEN_GPIO_Pin                       GPIO_Pin_2
#define OVERFILL_ALARM_LED_RED_GPIO_Port                        GPIOB
#define OVERFILL_ALARM_LED_RED_GPIO_Pin                         GPIO_Pin_3
#define INTERSTITIAL_ALARM_LED_GREEN_GPIO_Port                  GPIOB
#define INTERSTITIAL_ALARM_LED_GREEN_GPIO_Pin                   GPIO_Pin_4
#define INTERSTITIAL_ALARM_LED_RED_GPIO_Port                    GPIOB
#define INTERSTITIAL_ALARM_LED_RED_GPIO_Pin                     GPIO_Pin_5
#define LOW_LEVEL_ALARM_LED_GREEN_GPIO_Port                     GPIOB
#define LOW_LEVEL_ALARM_LED_GREEN_GPIO_Pin                      GPIO_Pin_6
#define LOW_LEVEL_ALARM_LED_RED_GPIO_Port                       GPIOB
#define LOW_LEVEL_ALARM_LED_RED_GPIO_Pin                        GPIO_Pin_7
#define SYSTEM_HEALTH_LED_GREEN_GPIO_Port                       GPIOC
#define SYSTEM_HEALTH_LED_GREEN_GPIO_Pin                        GPIO_Pin_2
#define SYSTEM_HEALTH_LED_RED_GPIO_Port                         GPIOC
#define SYSTEM_HEALTH_LED_RED_GPIO_Pin                          GPIO_Pin_3
#define PULSE_COUNTER_1_LED_GREEN_GPIO_Port                     GPIOB
#define PULSE_COUNTER_1_LED_GREEN_GPIO_Pin                      GPIO_Pin_0
#define PULSE_COUNTER_2_LED_GREEN_GPIO_Port                     GPIOB
#define PULSE_COUNTER_2_LED_GREEN_GPIO_Pin                      GPIO_Pin_1
#define BYPASS_1_LED_RED_GPIO_Port                              GPIOC
#define BYPASS_1_LED_RED_GPIO_Pin                               GPIO_Pin_4
#define BYPASS_2_LED_RED_GPIO_Port                              GPIOC
#define BYPASS_2_LED_RED_GPIO_Pin                               GPIO_Pin_5


/*
#define POWER_STATUS_LED_GREEN_GPIO_Port                        GPIOE
#define POWER_STATUS_LED_GREEN_GPIO_Pin                         GPIO_Pin_7
#define INTERNET_STATUS_LED_BLUE_GPIO_Port                      GPIOC
#define INTERNET_STATUS_LED_BLUE_GPIO_Pin                       GPIO_Pin_7
#define INTERNET_STATUS_LED_RED_GPIO_Port                       GPIOC
#define INTERNET_STATUS_LED_RED_GPIO_Pin                        GPIO_Pin_6
#define OVERFILL_ALARM_LED_GREEN_GPIO_Port                      GPIOC
#define OVERFILL_ALARM_LED_GREEN_GPIO_Pin                       GPIO_Pin_6
#define OVERFILL_ALARM_LED_RED_GPIO_Port                        GPIOC
#define OVERFILL_ALARM_LED_RED_GPIO_Pin                         GPIO_Pin_6
#define INTERSTITIAL_ALARM_LED_GREEN_GPIO_Port                  GPIOC
#define INTERSTITIAL_ALARM_LED_GREEN_GPIO_Pin                   GPIO_Pin_6
#define INTERSTITIAL_ALARM_LED_RED_GPIO_Port                    GPIOC
#define INTERSTITIAL_ALARM_LED_RED_GPIO_Pin                     GPIO_Pin_6
#define LOW_LEVEL_ALARM_LED_GREEN_GPIO_Port                     GPIOC
#define LOW_LEVEL_ALARM_LED_GREEN_GPIO_Pin                      GPIO_Pin_6
#define LOW_LEVEL_ALARM_LED_RED_GPIO_Port                       GPIOC
#define LOW_LEVEL_ALARM_LED_RED_GPIO_Pin                        GPIO_Pin_6
#define SYSTEM_HEALTH_LED_GREEN_GPIO_Port                       GPIOC
#define SYSTEM_HEALTH_LED_GREEN_GPIO_Pin                        GPIO_Pin_6
#define SYSTEM_HEALTH_LED_RED_GPIO_Port                         GPIOC
#define SYSTEM_HEALTH_LED_RED_GPIO_Pin                          GPIO_Pin_6
#define PULSE_COUNTER_1_LED_GREEN_GPIO_Port                     GPIOC
#define PULSE_COUNTER_1_LED_GREEN_GPIO_Pin                      GPIO_Pin_6
#define PULSE_COUNTER_2_LED_GREEN_GPIO_Port                     GPIOC
#define PULSE_COUNTER_2_LED_GREEN_GPIO_Pin                      GPIO_Pin_6
#define BYPASS_1_LED_RED_GPIO_Port                              GPIOC
#define BYPASS_1_LED_RED_GPIO_Pin                               GPIO_Pin_6
#define BYPASS_2_LED_RED_GPIO_Port                              GPIOC
#define BYPASS_2_LED_RED_GPIO_Pin                               GPIO_Pin_6
*/
/*
******************************************************************************
*                         TYPEDEFS & STRUCTS
******************************************************************************
*/

typedef struct 
{
  bool            is_on;
  uint16_t        ON_duration;
  uint16_t        OFF_duration;
  uint16_t        total_duration;
  uint16_t        tick_counter;
}   LED_Config;

/*
******************************************************************************
*                         LOCAL VARIABLES
******************************************************************************
*/

static LED_Config LEDs[LEDn];
static bool is_timer_activated = FALSE;

/*
******************************************************************************
*                         LOCAL PROTOTYPES
******************************************************************************
*/

static void LED_ON( LED_TypeDef led );
static void LED_OFF( LED_TypeDef led );
//static void LED_Toggle( LED_TypeDef led );
/*
******************************************************************************
*                         LOCAL TABLES
******************************************************************************
*/


GPIO_TypeDef* LED_GPIO_PORT[LEDn] = { INTERNET_STATUS_LED_BLUE_GPIO_Port,
                                      INTERNET_STATUS_LED_RED_GPIO_Port,
                                      OVERFILL_ALARM_LED_GREEN_GPIO_Port,
                                      OVERFILL_ALARM_LED_RED_GPIO_Port,
                                      INTERSTITIAL_ALARM_LED_GREEN_GPIO_Port,
                                      INTERSTITIAL_ALARM_LED_RED_GPIO_Port,
                                      LOW_LEVEL_ALARM_LED_GREEN_GPIO_Port,
                                      LOW_LEVEL_ALARM_LED_RED_GPIO_Port,
                                      SYSTEM_HEALTH_LED_GREEN_GPIO_Port,
                                      SYSTEM_HEALTH_LED_RED_GPIO_Port,
                                      PULSE_COUNTER_1_LED_GREEN_GPIO_Port,
                                      PULSE_COUNTER_2_LED_GREEN_GPIO_Port,
                                      BYPASS_1_LED_RED_GPIO_Port,
                                      BYPASS_2_LED_RED_GPIO_Port };

const uint16_t LED_GPIO_PIN[LEDn] = { INTERNET_STATUS_LED_BLUE_GPIO_Pin,
                                      INTERNET_STATUS_LED_RED_GPIO_Pin,
                                      OVERFILL_ALARM_LED_GREEN_GPIO_Pin,
                                      OVERFILL_ALARM_LED_RED_GPIO_Pin,
                                      INTERSTITIAL_ALARM_LED_GREEN_GPIO_Pin,
                                      INTERSTITIAL_ALARM_LED_RED_GPIO_Pin,
                                      LOW_LEVEL_ALARM_LED_GREEN_GPIO_Pin,
                                      LOW_LEVEL_ALARM_LED_RED_GPIO_Pin,
                                      SYSTEM_HEALTH_LED_GREEN_GPIO_Pin,
                                      SYSTEM_HEALTH_LED_RED_GPIO_Pin,
                                      PULSE_COUNTER_1_LED_GREEN_GPIO_Pin,
                                      PULSE_COUNTER_2_LED_GREEN_GPIO_Pin,
                                      BYPASS_1_LED_RED_GPIO_Pin,
                                      BYPASS_2_LED_RED_GPIO_Pin };

/*
******************************************************************************
*                         LOCAL FUNCTIONS
******************************************************************************
*/

bool LED_init( void )
{
  bool ret = FALSE;
  
  /* turn OFF all leds */
  for( u8 i = 0; i < (u8)LEDn; ++i )
  {
    GPIO_Init(LED_GPIO_PORT[i], LED_GPIO_PIN[i], GPIO_Mode_Out_PP_Low_Fast);
  }
  
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
  
  TIM1_DeInit();
  
  /* config timer with overflow ever 10 ms */
   //timer freq = (clock CPU/16) -> 1bit = 1uS -> 10000*1uS=10 mS
  TIM1_TimeBaseInit(16, TIM1_CounterMode_Up, TIMER_PERIOD, TIMER_REPETITION_COUNTER);
  
  TIM1_ITConfig(TIM1_IT_Update, ENABLE);
  
  memset(LEDs, 0, sizeof(LEDs));
  ret = TRUE;
  
  return ret;
}

/*
******************************************************************************
*                         GLOBAL FUNCTIONS
******************************************************************************
*/

/**
* @brief Sets LED.
* @details Sets the mode of operation of the LED.  
* 
* @param led LED that needs to be sets.
* @param mode LED mode (ON, OFF, Flash).
*      In ON or OFF mode only check ON_duration.
* @param ON_duration ON time, unit of LED_TIMER_TICK.
*      If ON_duration == 0 - 
* @param OFF_duration OFF time, unit of LED_TIMER_TICK.
*      If OFF_duration == 0 - 
* @param total_duration total duration, unit of LED_TIMER_TICK.
*      If total_duration == 0 - infinitely.
*/
void LED_set( LED_TypeDef led, LED_Mode mode, uint16_t ON_duration, uint16_t OFF_duration, uint16_t total_duration )
{ 
  bool need_to_start_timer = FALSE;
  
  if( led < LEDn && mode <= LED_MODE_FLASH )
  {
#ifdef  USE_SEFE_LED_INFC
    taskENTER_CRITICAL();
#endif
    
    if( mode == LED_MODE_FLASH )
    {
      if( ON_duration > 0 && OFF_duration > 0 )
      {
        LEDs[led].is_on              = TRUE;
        LEDs[led].ON_duration        = ON_duration / TIMER_PRESCALER;
        LEDs[led].OFF_duration       = OFF_duration / TIMER_PRESCALER;
        LEDs[led].total_duration     = total_duration / TIMER_PRESCALER;
        LEDs[led].tick_counter       = ON_duration / TIMER_PRESCALER;
        
        LED_ON(led);
       need_to_start_timer = TRUE;
      }
    }
    else
    {
    
      LEDs[led].ON_duration        = 0;
      LEDs[led].OFF_duration       = 0;
      LEDs[led].total_duration     = total_duration / TIMER_PRESCALER;;
      LEDs[led].tick_counter       = 0;
      
      if( mode == LED_MODE_ON )
      {
        LEDs[led].is_on = TRUE;
        LED_ON(led);
      }
      else
      {
        LEDs[led].is_on = FALSE;
        LED_OFF(led);
      }
      
      if( total_duration != 0 )
      {
        need_to_start_timer = TRUE;
      }
    }
    
    if( need_to_start_timer == TRUE && is_timer_activated == FALSE )
    {
      /* Start the LED timer */
      TIM1_Cmd(ENABLE);
      is_timer_activated = TRUE;
    }
#ifdef  USE_SEFE_LED_INFC
    taskEXIT_CRITICAL();
#endif
  }
  else
  {
  }
}

/**
* @brief Sets LED Pin to HIGH state
*/
static void LED_OFF( LED_TypeDef led )
{
  LED_GPIO_PORT[led]->ODR |= LED_GPIO_PIN[led];
}

/**
* @brief Sets LED Pin to LOW state
*/
static void LED_ON( LED_TypeDef led )
{
  LED_GPIO_PORT[led]->ODR &= ~(LED_GPIO_PIN[led]);
}

/**
* @brief Changes LED Pin state
*//*
static void LED_Toggle( LED_TypeDef led )
{
LED_GPIO_PORT[led]->ODR ^= LED_GPIO_PIN[led];
}*/


/**
* @brief Call Back function for check LEDs.
* @details [long description]
*/
void LED_timer_callback()
{
  u8 i;
  bool need_to_stop_timer = TRUE;
  
  for( i = 0; i < (u8)LEDn; ++i )
  {
    if( LEDs[i].total_duration != 0 )
    {
      if( --LEDs[i].total_duration == 0 )
      {
        if( LEDs[i].is_on == TRUE )
        {
          LEDs[i].is_on = FALSE;
          LED_OFF(i);
        }
        
        LEDs[i].tick_counter = 0;
        continue;
      }
      
      need_to_stop_timer = FALSE;
    }
    
    if( LEDs[i].tick_counter != 0 )
    {
      if( --LEDs[i].tick_counter == 0 )
      {
        if( LEDs[i].is_on == TRUE )
        {
          LEDs[i].tick_counter = LEDs[i].OFF_duration;
          LEDs[i].is_on        = FALSE;
          LED_OFF(i);
        }
        else
        {
          LEDs[i].tick_counter = LEDs[i].ON_duration;
          LEDs[i].is_on        = TRUE;
          LED_ON(i);
        }
      }
      need_to_stop_timer = FALSE;
    }
  }
  
  if( need_to_stop_timer == TRUE )
  {
    /* Stop the LED timer */
    TIM1_Cmd(DISABLE);
    is_timer_activated = FALSE;
  }
}

void LED_all_off(void)
{
    LED_set(INTERNET_STATUS_LED_BLUE, LED_MODE_OFF, 0, 0, 0);
    LED_set(INTERNET_STATUS_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(OVERFILL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(OVERFILL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(INTERSTITIAL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(INTERSTITIAL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(LOW_LEVEL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(LOW_LEVEL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(PULSE_COUNTER_1_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(PULSE_COUNTER_2_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
    LED_set(BYPASS_1_LED_RED, LED_MODE_OFF, 0, 0, 0);
    LED_set(BYPASS_2_LED_RED, LED_MODE_OFF, 0, 0, 0);
}

void LED_parse(LED_TypeDef led, uint8_t mode)
{
  switch(led)
  {
    /* Internet connection LED */
  case INTERNET_STATUS_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(INTERNET_STATUS_LED_BLUE, LED_MODE_OFF, 0, 0, 0);
          LED_set(INTERNET_STATUS_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1: 
        {
          LED_set(INTERNET_STATUS_LED_BLUE, LED_MODE_ON, 0, 0, 0);
          LED_set(INTERNET_STATUS_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 2: 
        {
          LED_set(INTERNET_STATUS_LED_BLUE, LED_MODE_OFF, 0, 0, 0);
          LED_set(INTERNET_STATUS_LED_RED, LED_MODE_ON, 0, 0, 0);
          break;
        }
      case 3: 
        {
          LED_set(INTERNET_STATUS_LED_BLUE, LED_MODE_FLASH, 1500, 1500, 0);
          LED_set(INTERNET_STATUS_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      default:
        break;
      }
      //     i += 2;
      break;
    }
    /* Overfill alarm LED */
  case OVERFILL_ALARM_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(OVERFILL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(OVERFILL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1:
        {
          LED_set(OVERFILL_ALARM_LED_GREEN, LED_MODE_ON, 0, 0, 0);
          LED_set(OVERFILL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 2:
        {
          LED_set(OVERFILL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(OVERFILL_ALARM_LED_RED, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      default:
        break;
      }
      //     i += 2;
      break;
    }
    /* Interstitial alarm LED */
  case INTERSTITIAL_ALARM_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(INTERSTITIAL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(INTERSTITIAL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1:
        {
          LED_set(INTERSTITIAL_ALARM_LED_GREEN, LED_MODE_ON, 0, 0, 0);
          LED_set(INTERSTITIAL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 2:
        {
          LED_set(INTERSTITIAL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(INTERSTITIAL_ALARM_LED_RED, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      default:
        break;
      }
      //i += 2;
      break;
    }
    /* Low level alarm LED */
  case LOW_LEVEL_ALARM_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(LOW_LEVEL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(LOW_LEVEL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1:
        {
          LED_set(LOW_LEVEL_ALARM_LED_GREEN, LED_MODE_ON, 0, 0, 0);
          LED_set(LOW_LEVEL_ALARM_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 2:
        {
          LED_set(LOW_LEVEL_ALARM_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(LOW_LEVEL_ALARM_LED_RED, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      default:
        break;
      }
      //      i += 2;
      break;
    }
    /* System health LED */
  case SYSTEM_HEALTH_LED:
    {
      switch(mode)
      {
      case 1:
        {
          LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_ON, 0, 0, 0);
          LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 2:
        {
          LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      case 3:
        {
          LED_set(SYSTEM_HEALTH_LED_GREEN, LED_MODE_FLASH, 1500, 1500, 0);
          LED_set(SYSTEM_HEALTH_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      default:
        break;
      }
      //     i += 2;
      break;
    }
    /* Pulse counter 1 LED */
  case PULSE_COUNTER_1_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(PULSE_COUNTER_1_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1:
        {
          LED_set(PULSE_COUNTER_1_LED_GREEN, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      default:
        break;
      }
      //    i += 2;
      break;
    }
    /* Pulse counter 2 LED */
  case PULSE_COUNTER_2_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(PULSE_COUNTER_2_LED_GREEN, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1:
        {
          LED_set(PULSE_COUNTER_2_LED_GREEN, LED_MODE_FLASH, 1500, 1500, 0);
          break;
        }
      default:
        break;
      }
      //      i += 2;
      break;
    }
    /* Bypass 1 LED */
  case BYPASS_1_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(BYPASS_1_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1: 
        {
          LED_set(BYPASS_1_LED_RED, LED_MODE_ON, 0, 0, 0);
          break;
        }
      default:
        break;
      }
      //     i += 2;
      break;
    }
    /* Bypass 2 LED */
  case BYPASS_2_LED:
    {
      switch(mode)
      {
      case 0:
        {
          LED_set(BYPASS_2_LED_RED, LED_MODE_OFF, 0, 0, 0);
          break;
        }
      case 1: 
        {
          LED_set(BYPASS_2_LED_RED, LED_MODE_ON, 0, 0, 0);
          break;
        }
      default:
        break;
      }
      //     i += 2;
      break;
    }
  }
  
}
