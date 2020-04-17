// ****************************************************************************
//
//  led.h
//
//
// ****************************************************************************

#ifndef __LED_H
#define __LED_H

/*
 ******************************************************************************
 *                         INCLUDES
 ******************************************************************************
 */
//#include "main.h"

//#include "cmsis_os.h"

#include "stm8l15x_conf.h"


/*
 ******************************************************************************
 *                         TYPEDEFS & STRUCTS
 ******************************************************************************
 */

typedef enum
{   
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_FLASH,
    LED_MODE_NONE
}   LED_Mode;

typedef enum {
  INTERNET_STATUS_LED = 0,
  OVERFILL_ALARM_LED,
  INTERSTITIAL_ALARM_LED,
  LOW_LEVEL_ALARM_LED,
  SYSTEM_HEALTH_LED,
  PULSE_COUNTER_1_LED,
  PULSE_COUNTER_2_LED,
  BYPASS_1_LED,
  BYPASS_2_LED  
} LED_CodeTypeDef;

typedef enum 
{
    INTERNET_STATUS_LED_BLUE = 0,
    INTERNET_STATUS_LED_RED,
    OVERFILL_ALARM_LED_GREEN,
    OVERFILL_ALARM_LED_RED,
    INTERSTITIAL_ALARM_LED_GREEN,
    INTERSTITIAL_ALARM_LED_RED,
    LOW_LEVEL_ALARM_LED_GREEN,
    LOW_LEVEL_ALARM_LED_RED,
    SYSTEM_HEALTH_LED_GREEN,
    SYSTEM_HEALTH_LED_RED,
    PULSE_COUNTER_1_LED_GREEN,
    PULSE_COUNTER_2_LED_GREEN,
    BYPASS_1_LED_RED,
    BYPASS_2_LED_RED,
    LEDn // total led                   
}   LED_TypeDef;

/*
 ******************************************************************************
 *                         DEFINES
 ******************************************************************************
 */
     

/*
 ******************************************************************************
 *                         GLOBAL PROTOTYPES
 ******************************************************************************
 */

bool LED_init( void );
void LED_set( LED_TypeDef, LED_Mode, uint16_t, uint16_t, uint16_t );
void LED_parse(LED_TypeDef, uint8_t);
void LED_timer_callback();
void LED_all_off(void);

#endif /* __LED_H */
