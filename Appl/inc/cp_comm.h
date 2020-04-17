/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CP_COMM_H
#define __CP_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"

#define MAX_RX_BUFFER 253
#define MAX_TX_BUFFER 20

#define I2C_SPEED 100000u   /*speed in Hz*/

#define START_BYTE0     (u8)0xAA
#define START_BYTE1     (u8)0x55

typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;
     
typedef enum 
{
    NACK = 0,
    ACK = 0xA
}acknowledgment;
     
typedef enum
{
  GET_VER_APL = 1,
  SEND_PART_APP,
  START_UPDATE,
  RUN_APL,
  SET_LEDS,
  DO_RESET,
  GET_COUNTER1,
  GET_COUNTER2,
  SET_COUNTER1,
  SET_COUNTER2,
  GET_OPEN_BOX_STATUS,
  SYS_OK
}instruction_TypeDef;

typedef enum 
{
  APPLICATION_READY = 1,
  APPLICATION_NOT_FOUND,
  APPLICATION_UPDATE
}app_status_TypeDef;

typedef struct 
{
  bool ready;
  bool start_cnd;
  instruction_TypeDef instruction;
  uint8_t size_of_data;
  uint16_t check_summ;
  uint8_t *ptr_data;
  uint8_t temp_cnt;
}I2C_received_packet_TypeDef;

typedef struct 
{
  uint8_t number_of_bytes_sent;
  uint8_t *ptr_data_send;
}I2C_transmitted_packet_TypeDef;

extern bool FirmwareUpdate;
extern u16 size_of_appl;
extern uint8_t rx_Buffer[];
extern uint8_t tx_Buffer[];

extern I2C_received_packet_TypeDef I2C_received_packet;
extern I2C_transmitted_packet_TypeDef I2C_transmitted_packet;

/* Exported functions ------------------------------------------------------- */
    
void CP_COMM_Init();
void CP_COMM_GetData(uint16_t size, uint8_t* data);
void CP_COMM_SendData();

void I2C_transaction_begin(void);
void I2C_transaction_end(void);
uint8_t I2C_byte_write(void);
void I2C_byte_received(u8 u8_RxData);
uint8_t I2C_SendByte(void);
bool check_summ(void);


#endif /* __CP_COMM_H */

/**
  * @}
  */
  
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
