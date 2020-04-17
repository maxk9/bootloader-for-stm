/**
  ******************************************************************************
  * File main.c
  * Description: Example of implementation of user bootloader (for Application Note AN2659)
  * Author: STMicroelectronics - MCD Prague Application Team
  * Version V2.0.0
  * Date 25/11/2010
  * Product: STM8L
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * COPYRIGHT 2008 STMicroelectronics
  ******************************************************************************
  */ 
	
#include "main.h"

#define RELOAD_VALUE   254


//typedef FAR void (*)(void) TFunction;
typedef  void (*TFunction)(void);

//main application code (user reset) - init user code start - to interrupt table reset jump
const TFunction MainUserApplication = (TFunction)MAIN_USER_RESET_ADDR;
//address for GO command
TFunction GoAddress;

//Declare input buffer and its pointer 
u8 DataBuffer[150];
u8 TransmittedData[5];
//Declare loaded into RAM
u8 RoutinesInRAM = 0;
u16 size_of_appl;

u8 fmw_update_started = 0;

I2C_received_packet_TypeDef I2C_received_packet = {FALSE, FALSE, 0, 0, 0, DataBuffer, 0};
I2C_transmitted_packet_TypeDef I2C_transmitted_packet = {0, TransmittedData};

void run_apl(void)
{
    if((*((u8 FAR*)MainUserApplication)==0x82) || (*((u8 FAR*)MainUserApplication)==0xAC))
    {
        DeInitBootloader();
        
        /* IWDG Configuration */
        IWDG_Config();
        
        //reset stack pointer (lower byte - because compiler decreases SP with some bytes)			
         asm("LDW X,  SP ");
         asm("LD  A,  $FF");
         asm("LD  XL, A  ");
         asm("LDW SP, X  ");
         asm("JPF $9000");
    }
}



//**************************************************************************
void main(void){
  u8 I2C_read_data;
  
  sim();    // disable interrupts 
  
  RoutinesInRAM = 0;
  
  /* Set High speed internal clock prescaler */
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1); /* main clk is 16MHz */

  GPIO_Init(GPIOA, GPIO_Pin_All, GPIO_Mode_In_PU_No_IT);
  GPIO_Init(GPIOB, GPIO_Pin_All, GPIO_Mode_In_PU_No_IT);
  GPIO_Init(GPIOD, GPIO_Pin_All, GPIO_Mode_In_PU_No_IT);
  GPIO_Init(GPIOC, GPIO_Pin_All, GPIO_Mode_In_PU_No_IT);
   
  /* initialisation of selected protocol peripheral	*/
  protocol_init();

  //wait for instruction
  while(1u)
  {
     I2C_wait_packet(); /* run appl automaticaly after ~5sec */
    
     if(check_summ() == TRUE)
      {
        switch( I2C_received_packet.instruction )
        {
            case START_UPDATE:            
                /* get size */
                size_of_appl = ((u16)I2C_received_packet.ptr_data[0]<<8) | I2C_received_packet.ptr_data[1];
                
                // copy routines to RAM
                RoutinesInRAM = 1;
              
                // send ACK to host
               I2C_transmitted_packet.ptr_data_send[0] = (u8)ACK;
               I2C_transmitted_packet.number_of_bytes_sent = 1;
                
               run_update_application(size_of_appl); /* loop until the whole application gets or will get DO_RESET */
               
            break;

            case RUN_APL:
                
                run_apl();
                
                I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
                I2C_transmitted_packet.number_of_bytes_sent = 1;
            break;

            case SEND_PART_APP: /* must be after START_UPDATE */
                I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
                I2C_transmitted_packet.number_of_bytes_sent = 1;
                break;

            default: 
                I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
                I2C_transmitted_packet.number_of_bytes_sent = 1;
            break;
        } 
      }
      else
      {
          I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
          I2C_transmitted_packet.number_of_bytes_sent = 1;
      }
  }
}//main

/**
  * @brief  Configures the IWDG to generate a Reset if it is not refreshed at the
  *         correct time. 
  * @param  None
  * @retval None
  */
void IWDG_Config(void)
{
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable(); 
  
  /* IWDG timeout equal to 214 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  /* IWDG configuration: IWDG is clocked by LSI = 38KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  
  /* IWDG timeout equal to 214.7 ms (the timeout may varies due to LSI frequency dispersion) */
  /* IWDG timeout = (RELOAD_VALUE + 1) * Prescaler / LSI 
                  = (254 + 1) * 32 / 38 000 
                  = 214.7 ms */
  IWDG_SetReload((uint8_t)RELOAD_VALUE);
  
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
}

void DeInitBootloader(void){
  if(RoutinesInRAM)
  {
    /* Lock program memory */
    FLASH_Lock(FLASH_MemType_Program);
    /* Lock data memory */
    FLASH_Lock(FLASH_MemType_Data);
  }
	
  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);
  
  /* DeInit I2C */	
  I2C_DeInit(I2C1);
	
}//DeInitBootloader

void protocol_init (void) 
{

	/* --------------  Initialize the I2C peripheral ------------- */
  
  CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, ENABLE);
  I2C_DeInit(I2C1);
  
  GPIO_Init(GPIOC, (GPIO_Pin_0|GPIO_Pin_1), GPIO_Mode_In_PU_No_IT); /* I2C gpio */
  
  I2C_Init(I2C1, I2C_SPEED, (SLAVE_ADDRESS<<1), I2C_Mode_I2C, I2C_DutyCycle_2, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);
  
  I2C_Cmd(I2C1, ENABLE);
}


u8 Receive(u8* ReceivedData, u8* time_exp_ON)
{	
    u8 sr1;						// working copy of I2C_SR1
    u8 sr2;						// working copy of I2C_SR2
    u8 sr3;						// working copy of I2C_SR3
    u8 status = 0;
   static uint8_t tx_curr_byte = 0;
   u32 time_exp = 0;
   
    while(status == 0)
    {
        /* do timeout if not fmw update */
        if(fmw_update_started == 0)
            ++time_exp;
        
        // save the I2C registers configuration
        sr1 = I2C1->SR1;
        sr2 = I2C1->SR2;
        sr3 = I2C1->SR3;
            
    /* Communication error? */
      if (sr2 & (I2C_SR2_WUFH | I2C_SR2_OVR |I2C_SR2_ARLO |I2C_SR2_BERR))
      {		
        I2C1->CR2 |= I2C_CR2_STOP;  // stop communication - release the lines
        I2C1->SR2 = 0;				// clear all error flags
        return 0;
      }else
    /* More bytes received ? */
      if ((sr1 & (I2C_SR1_RXNE | I2C_SR1_BTF)) == (I2C_SR1_RXNE | I2C_SR1_BTF))
      {
        *ReceivedData = (u8)I2C1->DR;
        status = 0xFF;
        time_exp = 0;
      }else
    /* Byte received ? */
      if (sr1 & I2C_SR1_RXNE)
      {
        *ReceivedData = (u8)I2C1->DR;
        status = 0xFF;
        time_exp = 0;
      }else
    /* NAK? (=end of slave transmit comm) */
      if (sr2 & I2C_SR2_AF)
      {	
        I2C1->SR2 &= ~I2C_SR2_AF;	  // clear AF
        time_exp = 0;
      //  I2C_transaction_end();
      }else
    /* Stop bit from Master  (= end of slave receive comm) */
      if (sr1 & I2C_SR1_STOPF) 
      {
        I2C1->CR2 |= I2C_CR2_ACK;	  // CR2 write to clear STOPF
        time_exp = 0;
       // I2C_transaction_end();
      }else
    /* Slave address matched (= Start Comm) */
      if (sr1 & I2C_SR1_ADDR)
      {	 
        //I2C_transaction_begin();
          time_exp = 0;
      }else
    /* More bytes to transmit ? */ /* Byte to transmit ? */
      if (((sr1 & (I2C_SR1_TXE | I2C_SR1_BTF)) == (I2C_SR1_TXE | I2C_SR1_BTF)) || (sr1 & I2C_SR1_TXE))
      {
        if( I2C_transmitted_packet.number_of_bytes_sent > 0)
        {
            --I2C_transmitted_packet.number_of_bytes_sent;
            I2C1->DR = I2C_transmitted_packet.ptr_data_send[tx_curr_byte++];
        }
        else 
        {
            tx_curr_byte = 0;
            I2C1->DR = 0xFF;    /* nothing for send */
        }
        time_exp = 0;
      }
      
      if(time_exp > 1000000u)
      {
          *time_exp_ON = (u8)0xFF;
          break;
      }
    }
    return 1;
}//Receive

void I2C_wait_packet(void)
{
//    I2C_received_packet.ptr_data[I2C_received_packet.temp_cnt] = u8_RxData;
//    ++I2C_received_packet.temp_cnt;
  u8 timeout_expired = 0;
  static bool st_cn1 = FALSE;
  static bool st_cn0 = FALSE;
  static u8 amount_rx_bytes = 0;
  static bool got_instruction = FALSE;
  static bool got_size = FALSE;
  static bool got_check_summ = FALSE;
  u8 u8_RxData;
  
  I2C_received_packet.ready = 0;
  
  while(!I2C_received_packet.ready && (timeout_expired == 0))
  {
  
      if(Receive(&u8_RxData, &timeout_expired) == 0) /* error receiv */
         return;
      
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
      }
      else
      if(got_instruction == FALSE)
      {
        got_instruction = TRUE;
        /* get instruction */
        I2C_received_packet.instruction = u8_RxData;
      }
      else
      if(got_size == FALSE)
      {
        got_size = TRUE;
        I2C_received_packet.size_of_data = u8_RxData;
        amount_rx_bytes = 0; /* reset counter received bytes */
      }
      else
      if(amount_rx_bytes < I2C_received_packet.size_of_data)
      {
         I2C_received_packet.ptr_data[amount_rx_bytes++] = u8_RxData;
      }
      else
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
  
  if(timeout_expired == 0xFF)
      run_apl();
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

TestStatus programm_flash(u16 num_of_block, u8 *data)
{   
      uint32_t add, startadd, stopadd = 0;
      uint32_t i = 0;
      u16 curr_block = num_of_block + (APPLICATION_ADDRESS - FLASH_PROGRAM_START_PHYSICAL_ADDRESS)/FLASH_BLOCK_SIZE;
      /* This function is executed from RAM */
      FLASH_ProgramBlock(curr_block, FLASH_MemType_Program, FLASH_ProgramMode_Standard, data);
      
      /* Wait until End of high voltage flag is set*/
      while (FLASH_GetFlagStatus(FLASH_FLAG_HVOFF) == RESET)
      {}
      
      /* Check the programmed block */
      startadd = APPLICATION_ADDRESS + ((uint16_t)num_of_block * (uint16_t)FLASH_BLOCK_SIZE);
      stopadd = startadd + (uint16_t)FLASH_BLOCK_SIZE;
      
      for (add = startadd; add < stopadd; add++)
      {
        if (FLASH_ReadByte(add) != data[i])
        {
          /* Error */
          return FAILED;
        }
        i++;
      }
      
    /* Pass */
    return PASSED;
}


static void run_update_application(u16 size_of_appl)
{
    u16 curr_part_of_appl = 0;
    
    /* Define flash programming Time*/
    FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);

    FLASH_Unlock(FLASH_MemType_Program);
  
    /* Wait until Flash Program area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_PUL) == RESET)
    {}

    /* Unlock flash data eeprom memory */
    FLASH_Unlock(FLASH_MemType_Data);
  
    /* Wait until Data EEPROM area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {}          
    
    fmw_update_started = 0xFF;
    
    while(curr_part_of_appl < size_of_appl)
    {
         I2C_wait_packet();

          if(check_summ() == TRUE)
          {
            if( (I2C_received_packet.instruction == SEND_PART_APP) &&
               (I2C_received_packet.size_of_data == FLASH_BLOCK_SIZE)) /* got part of data equal FLASH_BLOCK_SIZE */
            {
                if(programm_flash(curr_part_of_appl, I2C_received_packet.ptr_data) == FAILED)
                {
                     I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
                     I2C_transmitted_packet.number_of_bytes_sent = 1;
                }
                else
                {
                    ++curr_part_of_appl;
                    
                    I2C_transmitted_packet.ptr_data_send[0] = (u8)ACK;
                    I2C_transmitted_packet.number_of_bytes_sent = 1;                
                }
            }
            else
            {
                if(I2C_received_packet.instruction == DO_RESET)
                {
                   break; /* interrupt firmware update proccess */
                }
                
                I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
                I2C_transmitted_packet.number_of_bytes_sent = 1;
            }
          }
          else
          {
              I2C_transmitted_packet.ptr_data_send[0] = (u8)NACK;
              I2C_transmitted_packet.number_of_bytes_sent = 1;
          }
    }
}

//**************************************************************************

