

#ifndef BOX_OPEN_H_
#define BOX_OPEN_H_


#include "stm8l15x.h"


#define BOX_OPEN_STATUS         0

extern void BOXSTS_Init();
extern uint8_t read_status_box_open();
extern void BOXSTS_Reset_switch_state();

inline void BOXSTS_SetAlarm(uint8_t state)
{
  uint8_t arr[FLASH_BLOCK_SIZE] = {0};
  arr[0] = state;
  
  /* Unlock flash data eeprom memory */
  FLASH_Unlock( FLASH_MemType_Data );
  /* Wait until Data EEPROM area unlocked flag is set*/
  while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
  {}

  FLASH_ProgramBlock( BOX_OPEN_STATUS, FLASH_MemType_Data, FLASH_ProgramMode_Standard, arr );
  
  /* Wait until End of high voltage flag is set*/
  while (FLASH_GetFlagStatus(FLASH_FLAG_HVOFF) == RESET)
  {}
  
  FLASH_Lock( FLASH_MemType_Data );
}

#endif /*BOX_OPEN_H_ */