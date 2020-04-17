/*
 * box_open.c
 *
 *  Created on: Nov 29, 2019
 *      Author: maximk
 */


#include "box_open.h"

#define BOX_SEN_STS_Pin GPIO_Pin_6
#define BOX_SEN_STS_GPIO_Port GPIOC


bool bBoxOpened = FALSE;

static uint8_t stat;

void BOXSTS_Init()
{
  GPIO_TypeDef GPIO_InitStruct = {0};
  
  EXTI_DeInit();
   
  disableInterrupts();
 
  stat = FLASH_ReadByte( FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + ((uint16_t)BOX_OPEN_STATUS * (uint16_t)FLASH_BLOCK_SIZE));
    
  /*Configure GPIO pins : BOX_SEN_STS_Pin */
  GPIO_Init( BOX_SEN_STS_GPIO_Port, BOX_SEN_STS_Pin, GPIO_Mode_In_FL_IT );
  
  EXTI_SetPinSensitivity(EXTI_Pin_6, EXTI_Trigger_Falling);
  
   /* Define flash programming Time*/
  FLASH_SetProgrammingTime( FLASH_ProgramTime_Standard );
  
  //DEBUG_INFO("Box open sensor initialization ended");
}

uint8_t read_status_box_open()
{
   return stat;    
}

