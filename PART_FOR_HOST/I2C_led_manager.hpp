/*
 * I2C_led_manager.hpp
 *
 *  Created on: Aug 16, 2019
 *      Author: maximk
 */

#pragma once


#include <string.h>
#include <iostream>
#include "pump.hpp"
#include "common.h"


using namespace std;

namespace I2C_mod
{
	typedef enum
	{
		NACK = 0, ACK = 0xA
	} acknowledgment;

	typedef enum
	{
		SET_OK = 1, SET_ERROR
	} I2C_status;

	/* !!!should be equal to settings in slave device!!! */
	typedef enum
	{
		LED_MODE_OFF, LED_MODE_ON, LED_MODE_FLASH
	} I2C_LED_Mode;

	/* !!!should be equal to settings in slave device!!! */
	typedef enum
	{
		INTERNET_STATUS_LED_GREEN = 0,
		INTERNET_STATUS_LED_RED,
		OVERFILL_ALARM_LED_GREEN,
		OVERFILL_ALARM_LED_RED,
		INTERSTITIAL_ALARM_LED_GREEN,	/* 4 */
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
	} I2C_LedType;

	/* !!!should be equal to settings in slave device!!! */
	typedef enum
	{
		GET_VER_APL = 1,
		SEND_PART_APP,
		START_UPDATE,
		RUN_APL,		/* 4 */
		SET_LEDS,		/* 5 */
		DO_RESET,		/* 6 */
		GET_COUNTER1,	/* 7 */
		GET_COUNTER2,	/* 8 */
		SET_COUNTER1,	/* 9 */
		SET_COUNTER2,
		GET_SWITCH_STATE,
		SYS_OK
	} I2C_instruction_TypeDef;

	struct I2C_packet
	{
		I2C_instruction_TypeDef instruction;
		uint8_t size_payload;
		uint8_t *ptr_to_data;
		uint16_t checksumm;

		I2C_packet& operator =(const I2C_packet& a)
		{
			instruction = a.instruction;
			size_payload = a.size_payload;
			ptr_to_data = a.ptr_to_data;
			checksumm = a.checksumm;
			return *this;
		}
	};

	class I2C
	{
	public:
		I2C(void);
		~I2C(void);
		static I2C& instance();
		int Tests(void);
		int Update_Firmware(string file_bin);
		int Check_Firmware(); // Checks firmware vresion and if the version does not correct it will update it
		int get_status(void) {return err;}
		I2C_status I2C_set_leds(I2C_LedType led, I2C_LED_Mode mode, uint16_t duration_ON, uint16_t duration_OFF, uint16_t duration_total);
		I2C_status I2C_get_counter(PumpId id_,uint8_t &over_cnt, uint16_t &counter);
		I2C_status I2C_get_counter(PumpId id_, uint32_t &counter);
		I2C_status I2C_reset_counter(PumpId id_);
		I2C_status I2C_get_version_appl(string *appl_version);
		I2C_status I2C_do_reset_run_bootloader(void);
		I2C_status I2C_get_switch_state(bool &state);
		I2C_status I2C_send_SYS_OK(void);

	private:
		string version = "";
		uint8_t status_ofON_led[LEDn] = {0xFF};
		int file_ = 0, err = 0;
		string I2C_name = "/dev/i2c-0";
		int I2C_slave_addr = 0x51;
		uint8_t packet_arr[150] = {0};
		bool fmw_update_started = false;

		pthread_mutexattr_t m_I2C_attr;
		pthread_mutex_t m_I2C;
		bool isInitialized = false;
		ModuleStatus send_packet(I2C_packet tx_packet, uint8_t cnt_read_data, uint8_t *rx_packet);
	};

	int I2C_common_test();

}
