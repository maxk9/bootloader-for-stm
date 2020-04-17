/**
  ******************************************************************************
  * @file main.h
  * @brief This file contains all definition for AN2659 user-Bootloader examples 
  * @author STMicroelectronics - APG Application Team
  * @version V2.0.0
  * @date 25/11/2010
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2008 STMicroelectronics</center></h2>
  * @image html logo.bmp
  ******************************************************************************
  */


#include "stm8l15x.h"

/*********************************************************/
/*              USER BOOT CODE PROTOCOL PARAMETERS	     */
/*********************************************************/


/*********************************************************/
/*              USER BOOT CODE Customisation 	     			 */
/*********************************************************/

//user application start (user interrupt table address)
#define MAIN_USER_RESET_ADDR 0x9000ul


#define I2C_SPEED 100000u   /*speed in KHz*/
// I2C - Adress (only if I2C protocol used)
#define SLAVE_ADDRESS 0x51

/*********************************************************/
/*              USER BOOT CODE MEMORY PARAMETERS	       */
/*********************************************************/

/* Parameters of this section depend of the used Microcontroler */
/* see microcontroler's data sheet for more information 				*/

#define SIZE_OF_BOOTLOADER          32u /* size of bootloader 32 blocks x 128bytes */

#define APPLICATION_STATUS        (uint16_t)0x8800
#define APPLICATION_ADDRESS       (uint16_t)0x9000

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
  SET_COUNTER2
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


void DeInitBootloader(void);
void protocol_init(void);
u8 Receive(u8* ReceivedData, u8* time_exp_ON);

void I2C_wait_packet(void);
bool check_summ(void);
TestStatus programm_flash(u16 num_of_block, u8 *data);
void run_update_application(u16 size_of_appl);
void IWDG_Config(void);
/* Exported types ------------------------------------------------------------*/


