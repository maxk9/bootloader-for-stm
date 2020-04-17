/* Includes ------------------------------------------------------------------*/
#include "cp_comm.h"
#include "led.h"

/** @addtogroup STM8L15x_StdPeriph_Driver
* @{
*/

/** @defgroup I2C
* @brief I2C driver modules
* @{
*/

/* Private define ------------------------------------------------------------*/
#define LED_MASTER_INIT 0x40

#define LED_READY       0x51
#define LED_ERROR       0x52
#define LED_SUCCESS     0x54
#define LED_EQUAL       0x58
#define LED_VERSION     0x5E

#define MASTER_ADDRESS 0x10
#define SLAVE_ADDRESS 0x51


uint8_t rx_Buffer[MAX_RX_BUFFER];
uint8_t tx_Buffer[MAX_TX_BUFFER];

I2C_received_packet_TypeDef I2C_received_packet = {FALSE, FALSE, 0, 0, 0, rx_Buffer, 0};
I2C_transmitted_packet_TypeDef I2C_transmitted_packet = {0, tx_Buffer};


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

bool FirstByte;
bool SecondByte;
bool ThirdByte;
bool Version;
bool MasterExists;
uint8_t versionCount;
uint8_t msgIndex;

//extern uint8_t applicationStatus;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup I2C_Private_Functions
* @{
*/

/* under interrupt !!! */
uint8_t I2C_byte_write(void)
{
  static uint8_t curr_byte = 0;
  if( I2C_transmitted_packet.number_of_bytes_sent > 0)
  {
    --I2C_transmitted_packet.number_of_bytes_sent;
    return I2C_transmitted_packet.ptr_data_send[curr_byte++];
  }
  else 
  {
    curr_byte = 0;
    return 0xFF;    /* nothing for send */
  }
}

void I2C_transaction_begin(void)
{

}

void I2C_transaction_end(void)
{

}

/* under interrupt !!! */
void I2C_byte_received(u8 u8_RxData)
{
//    I2C_received_packet.ptr_data[I2C_received_packet.temp_cnt] = u8_RxData;
//    ++I2C_received_packet.temp_cnt;
    
  static bool st_cn1 = FALSE;
  static bool st_cn0 = FALSE;
  static u8 amount_rx_bytes = 0;
  static bool got_instruction = FALSE;
  static bool got_size = FALSE;
  static bool got_check_summ = FALSE;
     
  if( I2C_received_packet.ptr_data == 0 ) /* input buffer not defined */
    return; 
  
  /* check start condition */
  if( (st_cn0 == FALSE) || (st_cn1 == FALSE) )
  {
    if( u8_RxData == START_BYTE0 )
    {
        st_cn0 = TRUE;
    }
    else
    {
        if((st_cn0 == TRUE) && (u8_RxData == START_BYTE1))
        {
            st_cn1 = TRUE;
        }
        else
        {
            st_cn0 = FALSE;
        }
    }
    return;
  }
  
  if(got_instruction == FALSE)
  {
    got_instruction = TRUE;
    /* get instruction */
    I2C_received_packet.instruction = u8_RxData;
    return;
  }
  
  if(got_size == FALSE)
  {
    got_size = TRUE;
    I2C_received_packet.size_of_data = u8_RxData;
    amount_rx_bytes = 0; /* reset counter received bytes */
    return;
  }
  
  if(amount_rx_bytes < I2C_received_packet.size_of_data)
  {
     I2C_received_packet.ptr_data[amount_rx_bytes++] = u8_RxData;
     return;
  }
  
  if(got_check_summ == FALSE)
  {
    got_check_summ = TRUE;
    I2C_received_packet.check_summ = u8_RxData<<8;
  }
  else
   /* last byte for check summ */
  {
    I2C_received_packet.check_summ |= u8_RxData;
    I2C_received_packet.ready = TRUE;
    got_instruction = FALSE;
    got_size = FALSE;
    got_check_summ = FALSE;
    st_cn0 = FALSE;
    st_cn1 = FALSE;
  }
}


void CP_COMM_Init()
{
  CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, ENABLE);
  I2C_DeInit(I2C1);
  
  GPIO_Init(GPIOC, (GPIO_Pin_0|GPIO_Pin_1), GPIO_Mode_In_PU_No_IT); /* I2C gpio */
  
  I2C_Init(I2C1, I2C_SPEED, (SLAVE_ADDRESS<<1), I2C_Mode_I2C, I2C_DutyCycle_2, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);
  
  I2C_ITConfig(I2C1, (I2C_IT_ERR|I2C_IT_EVT|I2C_IT_BUF), ENABLE);
  
  I2C_Cmd(I2C1, ENABLE);
}

bool check_summ(void)
{
    u16 calc_summ = 0;
    u8 cnt = I2C_received_packet.size_of_data;
    calc_summ = (u16)START_BYTE0 + START_BYTE1 + (u8)I2C_received_packet.instruction + I2C_received_packet.size_of_data;
    
    while(cnt--)
    {
       calc_summ += I2C_received_packet.ptr_data[cnt];
    }
    
    if(calc_summ == I2C_received_packet.check_summ)
        return TRUE;
    else
        return FALSE;
}
/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
