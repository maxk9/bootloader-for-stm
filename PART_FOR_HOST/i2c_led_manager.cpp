/*
 * I2C_led_manager.cpp
 *
 *  Created on: Aug 16, 2019
 *      Author: maximk
 */


#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include "config_mgr.h"
#include <iterator>     // std::advance
#include "I2C_led_manager.hpp"
#include <thread>
#include <mutex>
#include <vector>

#include "main.hpp"
#include "log.h"
#include "firmware_update_mgr.hpp"
#include <cstdarg>
#include <alloca.h>

//#define DEBUG_OUT
//#define EXEC_TESTS 1
#undef EXEC_TESTS

#define MAX_SIZE 255
static mutex WriteMutex;
using namespace I2C_mod;
using namespace std;


extern const std::string software_ver;

static vector<string> split(const string& s, char d)
{
    vector<string> r;
    uint32_t j = 0;
    for (uint32_t i = 0; i < s.length(); i++)
    {
        if (s[i] == d)
        {
            r.push_back(s.substr(j, i - j));
            j = i + 1;
        }
    }
    r.push_back(s.substr(j));
    return r;
}

/* create instance for I2C service */
I2C& I2C::instance()
{
	static I2C I2C_serv;

	return I2C_serv;
}

int I2C_mod::I2C_common_test()
{
	I2C i2c_test;
	return i2c_test.Tests();
}

/* common send packet routine
 * input:
 * 		transmitted packet
 * 		count of data
 * received:
 * 		rx_packet - pointer of read ack data */
ModuleStatus I2C::send_packet(I2C_packet tx_packet, uint8_t cnt_read_data, uint8_t *rx_packet)
{
	uint8_t cnt=0;
	uint8_t i=0;
	uint16_t summ=0;
	uint8_t buf[MAX_SIZE] = {0};	// = {0xAA, 0x55, 1, 0, 1, 0};	/* request version */
	uint8_t read_buf[100] = {0};

	buf[0] = 0xAA;
	buf[1] = 0x55;
	buf[2] = (uint8_t)tx_packet.instruction;
	buf[3] = tx_packet.size_payload;

	while(tx_packet.size_payload--)
	{
		if(tx_packet.ptr_to_data != NULL)
		{
			buf[4+cnt]=tx_packet.ptr_to_data[cnt];
		}
		else
		{
			return ModuleStatus::ERROR;
		}

		++cnt;
	}

	while( i< (cnt + 4))
	{
		summ += buf[i];
		++i;
	}

	buf[i] = (summ>>8)&0xFF;
	++i;
	buf[i] = (summ)&0xFF;
	++i;

	WriteMutex.lock();
try
{
	if(file_ >= 0)
	{
//		if ((cur = lseek(file, 0, SEEK_SET)) < 0)
//		{
//			WriteMutex.unlock();
//			LOG_ERROR_func(ELogName::I2C,"Offset corrente: %d Error: %s", (int) lseek(file, 0, SEEK_CUR), strerror(errno));
//			return ModuleStatus::ERROR;
//		}
		lseek(file_, 0, SEEK_SET);
		if ((write(file_, buf, i)) != i)
		{
			 WriteMutex.unlock();
		   /* ERROR HANDLING: i2c transaction failed */
		   LOG_ERROR_func(ELogName::I2C, "Failed to write to the i2c bus. %s FD: %d ", strerror(errno), file_);
		   return ModuleStatus::ERROR;
		}
		else
		{
#if EXEC_TESTS == 1
		   cout<<"\nI2C:<< send data: ";
		   for(int y = 0; y < i; y++)
			   printf("%02X", buf[y]);
		   cout<<endl;
#endif
		}
	}
	else
		{
			WriteMutex.unlock();
			LOG_ERROR_func(ELogName::I2C, "descriptor file I2C corrupted %d", file_);
			return ModuleStatus::ERROR;
		}

	// Using I2C Read
//	if ((cur = lseek(file, 0, SEEK_SET)) < 0)
//	{
//		WriteMutex.unlock();
//		LOG_ERROR_func(ELogName::I2C,"Offset corrente: %d Error: lseek %s", (int) lseek(file, 0, SEEK_CUR), strerror(errno));
//		return ModuleStatus::ERROR;
//	}

	int read_data = 0;
	lseek(file_, 0, SEEK_SET);
	if ((read_data = read(file_, read_buf, cnt_read_data)) != cnt_read_data)
	{
		if(tx_packet.instruction != RUN_APL)
		{
			char c_arr[5];
			string s_send_hex, s_read_hex;

			/* ERROR HANDLING: i2c transaction failed */
			LOG_ERROR_func(ELogName::I2C,"Failed to read from the i2c bus. %s", strerror(errno));
			for (int y = 0; y < i; y++)
			{
				sprintf( c_arr,"%02X ", buf[y]);
				s_send_hex += c_arr;
			}
			for (int i = 0; i < read_data; i++)
			{
				sprintf( c_arr,"%02X ", read_buf[i]);
				s_read_hex += c_arr;
			}

			LOG_ERROR_func(ELogName::I2C,"expected read_data %d, actual read: %d, send data: %s, receive data %s", cnt_read_data, read_data, s_send_hex.c_str(), s_read_hex.c_str());

			WriteMutex.unlock();
			return ModuleStatus::ERROR;
		}
	}
	else
	{
		for(int i=0; i < read_data; i++)
		{
			rx_packet[i] = read_buf[i];
		}

#if EXEC_TESTS == 1
		printf("OK: I2C:>> read: %d expect: %d| ", read_data, cnt_read_data);
		for(int i=0; i < read_data; i++)
		   printf("%02X ", rx_packet[i]);

		if(cnt_read_data == 1)
			printf((rx_packet[0] == (uint8_t)ACK ? " ACK" : " NACK"));

		cout<<endl;
#endif
	}


}catch (...)
{
	LOG_ERROR_func(ELogName::I2C,"Failed operate i2c bus. %s", strerror(errno));
}
	WriteMutex.unlock();
	return ModuleStatus::OK;
}

template<typename Vector>
auto split_vector(const Vector& v, unsigned number_lines) {
  using Iterator = typename Vector::const_iterator;
  std::vector<Vector> rtn;
  Iterator it = v.cbegin();
  const Iterator end = v.cend();

  while (it != end) {
    Vector v;
    std::back_insert_iterator<Vector> inserter(v);
    const auto num_to_copy = std::min(static_cast<unsigned>(
        std::distance(it, end)), number_lines);
    std::copy(it, it + num_to_copy, inserter);
    rtn.push_back(std::move(v));
    std::advance(it, num_to_copy);
  }

  return rtn;
}

/* set leds common routine
 * all durations are in ms
 * duration_total - if it is 0 - infinite
 * */
I2C_status I2C::I2C_set_leds(I2C_LedType led, I2C_LED_Mode mode, uint16_t duration_ON, uint16_t duration_OFF, uint16_t duration_total)
{
	/* check that the leds didn't alredy set, for avoid redundance communication via i2c */
	if(!fmw_update_started)
	{
//		status_ofON_led[led] = (uint8_t)mode;

		uint8_t read_ACK = 0;
		I2C_packet packet = {SET_LEDS, 0, packet_arr, 0 };/* request set leds */

		packet.size_payload = 8;
		packet.ptr_to_data[0] = (uint8_t)led;
		packet.ptr_to_data[1] = (uint8_t)mode;

		packet.ptr_to_data[2] = (uint8_t)(duration_ON>>8);
		packet.ptr_to_data[3] = (uint8_t)duration_ON & 0xFF;
		packet.ptr_to_data[4] = (uint8_t)(duration_OFF>>8);
		packet.ptr_to_data[5] = (uint8_t)duration_OFF & 0xFF;
		packet.ptr_to_data[6] = (uint8_t)(duration_total>>8);
		packet.ptr_to_data[7] = (uint8_t)duration_total & 0xFF;

		if( send_packet(packet, 1, &read_ACK) == ModuleStatus::OK)
		{
			if(read_ACK == ACK)
				return SET_OK;
		}
		else
			return SET_ERROR;
	}

	return SET_OK;
}

/* get counter value from pulse counter
 * counter - current value
 * over_cnt - value of overflowed counter
 * */
I2C_status I2C::I2C_get_counter(PumpId id_,uint8_t &over_cnt, uint16_t &counter)
{
	uint8_t get_cnt[10] = {0};
	I2C_instruction_TypeDef instruction_cnt = ((id_ == PumpId::PUMP_1) ? GET_COUNTER1 : GET_COUNTER2);

	I2C_packet packet = {instruction_cnt, 0, packet_arr, 0 };/* request get counter */

	if( send_packet(packet, 3, get_cnt) == ModuleStatus::OK)
	{
		over_cnt = get_cnt[0];
		counter = get_cnt[1]<<8;
		counter |= get_cnt[2];

		return SET_OK;
	}
	else
		return SET_ERROR;
}

I2C_status I2C::I2C_get_counter(PumpId id_, uint32_t &counter)
{
    uint8_t over = 0;
    uint16_t get_cnt = 0;

    auto result = I2C_get_counter(id_,over, get_cnt);

    if( result == SET_OK )
    {
        counter = get_cnt + UINT16_MAX * (uint32_t)over;
    }

    return result;
}

I2C_status I2C::I2C_get_switch_state(bool &state)
{
	uint8_t get_cnt[10] = {0};
	I2C_packet packet = {GET_SWITCH_STATE, 0, packet_arr, 0 };/* request get counter */

	if( send_packet(packet, 1, get_cnt) == ModuleStatus::OK)
	{
		if(get_cnt[0] > 0)
			state = true;
		else
			state = false;

		return SET_OK;
	}
	else
	{
		return SET_ERROR;
	}
}

I2C_status I2C::I2C_send_SYS_OK(void)
{
	uint8_t read_ACK = 0;
	I2C_packet packet = {SYS_OK, 0, packet_arr, 0 };/* request get counter */

	send_packet(packet, 1, &read_ACK);

	return SET_OK;
}

/* reset counter in pulse counter internal value */
I2C_status I2C::I2C_reset_counter(PumpId id_)
{
	uint8_t set_cnt[10] = {0};
	I2C_instruction_TypeDef instruction_cnt = ((id_ == PumpId::PUMP_1) ? SET_COUNTER1 : SET_COUNTER2);

	I2C_packet packet = {instruction_cnt, 2, packet_arr, 0 };/* request reset counter */

	packet.size_payload = 2;
	packet.ptr_to_data[0] = (uint8_t)0x0;
	packet.ptr_to_data[1] = (uint8_t)0x0;

	if( send_packet(packet, 1, set_cnt) == ModuleStatus::OK)
	{
		return SET_OK;
	}
	else
		return SET_ERROR;

}

I2C_status I2C::I2C_get_version_appl(string *appl_version)
{
	uint8_t get_ver[10] = {0};
	I2C_packet packet = {GET_VER_APL, 0, packet_arr, 0 };/* request get version */

	if( send_packet(packet, 4, get_ver) == ModuleStatus::OK)
	{
		string ver((char*)get_ver);
		*appl_version = ver;

		return SET_OK;
	}
	else
		return SET_ERROR;
}

I2C_status I2C::I2C_do_reset_run_bootloader(void)
{
	uint8_t read_ACK;

	LOG_DEBUG_func(ELogName::I2C,"reset apl... run btl");

	I2C_packet packet = {DO_RESET, 0, packet_arr, 0 }; /* request get version */

	if( send_packet(packet, 1, &read_ACK) == ModuleStatus::OK)
	{
		this_thread::sleep_for(chrono::seconds(1));
		if(read_ACK == ACK)
			return SET_OK;
	}

	this_thread::sleep_for(chrono::seconds(1));
	return SET_ERROR;
}

int I2C::Update_Firmware(string file_bin)
{
	ifstream file_apl (file_bin, ios::binary);
	streampos begin,end;
	uint8_t read_ACK = NACK;

	if (!file_apl.is_open())
	{
		LOG_ERROR_func(ELogName::I2C,"Failed to open the FPC_mcu.bin, %s", strerror(errno));

		return SET_ERROR;
	}

	fmw_update_started = true;

	I2C_do_reset_run_bootloader();

	// copies all data into buffer
	vector<unsigned char> buffer(istreambuf_iterator<char>(file_apl), {});

	file_apl.close();

	LOG_DEBUG_func(ELogName::I2C,"FPC_mcu.bin opened, size: %d bytes",buffer.size());

	I2C_packet packet = {START_UPDATE, 2, packet_arr, 0 }; /* start update */

	uint16_t size_in_blocks = buffer.size()/128;
	uint8_t padded_bytes = (128 - buffer.size()%128);

//#if EXEC_TESTS == 1
	LOG_DEBUG_func(ELogName::I2C,"size in 128 blocks: %d and padded %d bytes",size_in_blocks, padded_bytes );
//#endif

	/* padding 0 to 128 */
	while(padded_bytes--)
	{
		buffer.push_back(0);
	}

	auto parts_128 = split_vector(buffer, 128);

	LOG_DEBUG_func(ELogName::I2C,"amount of pieces: %d",parts_128.size());

	packet.ptr_to_data[0] = (uint8_t)(parts_128.size()>>8);
	packet.ptr_to_data[1] = (uint8_t)((parts_128.size())&0xFF);

	if( send_packet(packet, 1, &read_ACK) != ModuleStatus::OK)
	{
		LOG_ERROR_func(ELogName::I2C,"noACK");
		return SET_ERROR;
	}

	if((read_ACK != ACK))
	{
		LOG_ERROR_func(ELogName::I2C,"noACK");
		return SET_ERROR;
	}

	packet.instruction = SEND_PART_APP;
	packet.size_payload = 128;

	vector<unsigned char>::iterator it = buffer.begin();

	/* send all application */
	int number_of_piece=0;

	for (auto piece : parts_128)
	{
		memcpy(packet.ptr_to_data, piece.data(), piece.size());

		LOG_DEBUG_func(ELogName::I2C,"number_of_piece: %d",number_of_piece);

		/* send part several times in case bad */
		int retry_send_piece = 30;

		while(--retry_send_piece)
		{
			if((send_packet(packet, 1, &read_ACK) == ModuleStatus::OK) && (read_ACK == ACK))
			{
				break;
			}
			else
			{
				LOG_ERROR_func(ELogName::I2C,"noACK retry %d" , retry_send_piece);
				//return SET_ERROR;
			}
		}

		/* error send */
		if(retry_send_piece == 0 )
		{
			LOG_ERROR_func(ELogName::I2C,"flashed %d from %d" , number_of_piece, parts_128.size());
			return SET_ERROR;
		}

		/* next of piece */
		++number_of_piece;
	}

	fmw_update_started = false;

	return 0; /* SUCCESS */
}

int I2C::Check_Firmware()
{
    int result = -1;

    try
    {
        // Check and update version of firmware if is n't correct
        std::string currentVersion;

        if( SET_OK == I2C_get_version_appl(&currentVersion) )
        {
            // TODO Change it !!
           // Create format what look like software_ver
            auto sfverArr = split(software_ver, '.');
            auto currentVersionNum = stoi(currentVersion);
            auto software_verNum = stoi(sfverArr.at(2));

            LOG_INFO_func( ELogName::I2C,"%s(): Version of pulse counter is %d", __func__ , currentVersionNum);

            if( currentVersionNum == software_verNum )
            {
                LOG_INFO_func( ELogName::I2C,"%s(): Version of pulse counter is correct", __func__ );
            }
            else
            {
                LOG_WARNING_func( ELogName::I2C,"%s(): Version of pulse counter is not correct. Pulse counter firmware will be updated",
                        __func__ );

                dt_hw_version hwid = {0,0};
                string path_pulse_counter = firmware_update::instance().getFilePath( hwid, dt_DevType::PulseCounter );

                if( I2C_mod::I2C::instance().Update_Firmware( path_pulse_counter ) == 0 )
                {
                	ModuleStatus status;
                	uint8_t read_ACK = 0;

                	I2C_packet packet = {RUN_APL, 0, packet_arr, 0 }; /* request run */

                    LOG_INFO_func( ELogName::I2C,"%s(): Firmware of pulse counter successfully updated", __func__ );

                	status = send_packet(packet, 1, &read_ACK);	/* run application */

                	if(status != ModuleStatus::OK)
                		return 1;

                	I2C_get_version_appl(&version);

                	LOG_INFO_func( ELogName::I2C,"version appl: %s", version.c_str());
                    result = 0;
                }
                else
                {
                    LOG_ERROR_func( ELogName::I2C,"%s(): Error updating firmware of pulse counter", __func__ );
                }
            }

        }
        else
        {
            LOG_ERROR_func( ELogName::I2C,"%s(): Error getting current version of pulse counter", __func__ );
        }
    }
    catch (exception const& e)
    {
        result = -1;
        LOG_ERROR_func( ELogName::I2C, "%s(): Error: %s", __func__, e.what() );
    }

    return result;
}
//#define first_prog

int I2C::Tests(void)
{
	int ret_value = 0;
	uint16_t cnt = 0;
	uint8_t ovr = 0;

#ifdef first_prog
	ModuleStatus status;
	uint8_t read_ACK = 0;
	I2C_packet packet = {RUN_APL, 0, packet_arr, 0 }; /* request run */

	ret_value = Update_Firmware("FPC_mcu.bin");

	if(ret_value != 0)
	{
		cout<<"\nI2C: wrong fmw update\n";
		return 1;
	}
	cout<<"I2C: fmw update SUCCESS\n";

	if(file_ < 0)
		{
			cout<< "error file closed"<<endl;
			return 0;
		}

	status = send_packet(packet, 1, &read_ACK);	/* run application */

	if(status != ModuleStatus::OK)
		return 1;

	I2C_get_version_appl(&version);

	cout<<"I2C: version appl: "<< version<<endl;

#else

//	I2C_do_reset_run_bootloader();
//	return 1;
//	this_thread::sleep_for(chrono::seconds(1));
	/* already run in constructor */
//	I2C_packet packet = { RUN_APL, 0, packet_arr, 0 }; /* request run */
//	/* run appl */
//	send_packet(packet, 1, &read_ACK);

//	this_thread::sleep_for(chrono::seconds(1));

	I2C_get_version_appl(&version);

	cout<<"\nI2C: version appl: "<<version<<"\n";

	if(I2C_get_counter(PumpId::PUMP_1, ovr, cnt) == SET_OK)
	{
		printf("I2C: counter1 Overflowed %d counter %d\n",ovr, cnt );
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter1 get error\n");
	}

	if(I2C_get_counter(PumpId::PUMP_2, ovr, cnt) == SET_OK)
	{
		printf("I2C: counter2 Overflowed %d counter %d\n",ovr, cnt );
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter2 get error\n");
	}


	if(I2C_reset_counter(PumpId::PUMP_1) == SET_OK)
	{
		printf("I2C: counter1 reseted\n");
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter1 get error\n");
	}

	if(I2C_reset_counter(PumpId::PUMP_2) == SET_OK)
	{
		printf("I2C: counter2 reseted\n");
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter2 get error\n");
	}

	if(I2C_get_counter(PumpId::PUMP_1, ovr, cnt) == SET_OK)
	{
		printf("I2C: counter1 Overflowed %d counter %d\n",ovr, cnt );
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter1 get error\n");
	}

	if(I2C_get_counter(PumpId::PUMP_2,ovr, cnt) == SET_OK)
	{
		printf("I2C: counter2 Overflowed %d counter %d\n",ovr, cnt );
	}
	else
	{
		ret_value += 1;
		printf("\nI2C: counter2 get error\n");
	}

	bool state = false;
	I2C_get_switch_state(state);

	if(state == true)
		cout<< "\nI2C: BOX had opened\n";
	else
		cout<< "\nI2C: BOX hadn't opened\n";
	uint16_t duration_ON = 10;
	uint16_t duration_OFF = 300;
	uint16_t duration_total = 0; /* infinite */

	printf("\nI2C: set led SYSTEM_HEALTH_LED_RED\n");

	I2C_set_leds(OVERFILL_ALARM_LED_RED, LED_MODE_FLASH, duration_ON, duration_OFF, duration_total);
//	this_thread::sleep_for(chrono::seconds(5));
//	I2C_do_reset_run_bootloader(); /* left in reset state */
#endif //first_prog else

    return ret_value;
}



I2C::I2C(void)
{
	unsigned int bufInt = 0;
	constexpr int BUF_SIZE = 300;
	char bufStr[BUF_SIZE]{ 0 };
	I2C_packet packet = {RUN_APL, 0, packet_arr, 0 }; /* request run */
	uint8_t read_ACK = 0;

	/* Get configuration */
	CfgMgr& cfgManager = CfgMgr::instance();

	/* Get heart beat value in sec */
	if (cfgManager.CfgGetSetting( CFG_I2C_NAME_OF_BLOCK, CFG_I2C_TITLE, CFG_I2C_DEV, &bufStr, libconfig::Setting::TypeString, sizeof(bufStr)) == true)
	{
		I2C_name = bufStr;

#if EXEC_TESTS == 1
		printf("\t%s: %s \n", CFG_I2C_DEV, I2C_name.c_str());
#endif

	}
	else
	{
		throw runtime_error(fmt("\nI2C: Parameter \"%s\" not found", CFG_I2C_DEV));
	}

	if (cfgManager.CfgGetSetting( CFG_I2C_NAME_OF_BLOCK, CFG_I2C_TITLE, CFG_I2C_SLAVE_ADDRESS, &bufInt, libconfig::Setting::TypeInt, sizeof(bufInt)) == true)
	{
		I2C_slave_addr = bufInt;
#if EXEC_TESTS == 1
		printf("\t%s: 0x%X \n", CFG_I2C_SLAVE_ADDRESS, bufInt);
#endif
	}
	else
	{
		throw runtime_error(fmt("\nI2C: Parameter \"%s\" not found", CFG_I2C_DEV));
	}

	while ((file_ = open(I2C_name.c_str(), O_RDWR)) < 0)
	{
		LOG_ERROR_func(ELogName::I2C,"Failed to open the bus. %s", strerror(errno));
	}

	LOG_INFO_func(ELogName::I2C,"open the bus %d", file_);

	{
		/* dummy read */
		char rx_packet[1];
		read(file_, rx_packet, 1);
	}

#if EXEC_TESTS == 1
	cout<<"I2C bus opened..."<<endl;
#endif

	if (ioctl(file_, I2C_SLAVE, I2C_slave_addr) < 0)
	{
		LOG_ERROR_func(ELogName::I2C,"Failed to acquire bus access and/or talk to slave. %s", strerror(errno));
		throw runtime_error( fmt( "\nI2C(): Error set slave address ", strerror(errno) ) );
	}

#if EXEC_TESTS == 1
	cout<<"I2C slave addr was set "<<endl;
	cout<<"\nI2C: send run APL\n";
#endif

#if EXEC_TESTS == 1
//	I2C_do_reset_run_bootloader();
	this_thread::sleep_for(chrono::seconds(1));
#else
	/* run appl */
	send_packet(packet, 1, &read_ACK);
#endif

	this_thread::sleep_for(chrono::seconds(1));
	LOG_DEBUG_func(ELogName::I2C,"I2C initialisation success");
}

I2C::~I2C(void)
{
#if EXEC_TESTS == 1
	cout<<"\nI2C: all data cleared\n";
#endif
	close(file_);
}





