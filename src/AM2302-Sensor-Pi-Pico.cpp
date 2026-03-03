/**
 * @file AM2302-Sensor-Pi-Pico.cpp
 * @author Frank Häfele (mail@frankhaefele.de)
 * @brief Measure Sensor Data of AM2302-Sensor with Pi Pico
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026 Frank Häfele
 * 
 */

#include <cstdio>
#include "pico/stdlib.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "AM2302-Sensor-Pi-Pico.h"

// AM2302 PIO program:
#include "AM2302.pio.h"

// initialize private static member
unsigned int AM2302::AM2302_Sensor::_offset = 0xFFFF;

/**
 * @brief Construct a new am2302::am2302 sensor::am2302 sensor object
 * 
 * @param pin Pin for AM2302 sensor
 */
AM2302::AM2302_Sensor::AM2302_Sensor(uint8_t pin) 
: _millis_last_read{0}
, _sm{pio_claim_unused_sm(_pio, true)}
, _pin{pin}
{
   if (AM2302::AM2302_Sensor::_offset == 0xFFFF) {
      AM2302::AM2302_Sensor::_offset = pio_add_program(_pio, &AM2302_program);
   }
}

/**
 * @brief begin function setup pin and run sensor check.
 * 
 * @return true if sensor check is successful.
 * @return false if sensor check failed.
 */
bool AM2302::AM2302_Sensor::begin() {
   // required delay() for a secure sensor check,
   // if you reset the mcu very fast one after another
   auto tic{time_us_64()};
   while ( time_us_64() - tic < READ_FREQUENCY * 1000U ) {
      sleep_ms(1U);
   }
   auto status{read()};
   _millis_last_read = time_us_64();
   if (status == AM2302_READ_OK) {
      return true;
   }
   else {
      return false;
   }
}

/**
 * @brief read functionality
 * 
 * @return sensor status
*/
int8_t AM2302::AM2302_Sensor::read() {
   auto status{read_sensor()};
   
   if (status == AM2302_READ_OK) {
      // return status immediately
      return status;
   }
   else if (status == AM2302_ERROR_READ_FREQ) {
      return status;
   }
   else if (status == AM2302_ERROR_TIMEOUT) {
      resetData();
      return status;
   }
   else if (status == AM2302_ERROR_CHECKSUM) {
      // nothing to do
      return status;
   }
   return status;
}


/**
 * @brief initiate start sequence and read sensor data
 * 
 * @return sensor status
*/
int8_t AM2302::AM2302_Sensor::read_sensor() {
   // check read frequency
   if ( time_us_64() - _millis_last_read < READ_FREQUENCY * 1000U) {
      return AM2302_ERROR_READ_FREQ;
   }
   _millis_last_read = time_us_64();
   
   // Init and start the state machine
   AM2302_program_init(_pio, _sm, _offset, _pin);
   // Start a reading
   pio_sm_put(_pio, _sm, 54);
   // Read 5 bytes
   uint8_t _data[5U] = {0};
   uint16_t wait_counter{0};
   
   while (pio_sm_is_rx_fifo_empty(_pio, _sm) && (wait_counter < READ_TIMEOUT) ) {
      ++wait_counter;
      sleep_us(1);
      if (wait_counter >= READ_TIMEOUT) {
         return AM2302_ERROR_TIMEOUT;
      }
   }

   for (size_t i = 0; i < 5U; ++i) {
      _data[i] = (uint8_t) pio_sm_get_blocking(_pio, _sm);
   }

   // Stop the state machine
   pio_sm_set_enabled(_pio, _sm, false);
   
   // check checksum
   _checksum_ok = (_data[4] == ( (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF) );

   if (_checksum_ok) {
      _hum  = static_cast<uint16_t>((_data[0] << 8) | _data[1]);
      if (_data[2] & 0x80) {
         // negative temperature detected
         _data[2] &= 0x7f;
         _temp = -static_cast<int16_t>((_data[2] << 8) | _data[3]);
      }
      else {
         _temp = static_cast<int16_t>((_data[2] << 8) | _data[3]);
      }
      return AM2302_READ_OK;
   }
   else {
      return AM2302_ERROR_CHECKSUM;
   }
}


/**
 * @brief get Sensor State in human readable manner
 * 
 * @return sensor state
*/
const char * AM2302::AM2302_Sensor::get_sensorState(int8_t state) const {
   if(state == AM2302_READ_OK) {
      return AM2302_STATE_OK;
   }
   else if(state == AM2302_ERROR_CHECKSUM) {
      return AM2302_STATE_ERR_CKSUM;
   }
   else if(state == AM2302_ERROR_TIMEOUT) {
      return AM2302_STATE_ERR_TIMEOUT;
   }
   else if(state == AM2302_ERROR_READ_FREQ) {
      return AM2302_STATE_ERR_READ_FREQ;
   }
   return AM2302_STATE_UNKNOWN;
}

/**
 * @brief reset temperature and humidity data
 * 
 */
void AM2302::AM2302_Sensor::resetData() {
   // reset tem to -255 and hum to 0 as indication
   _temp = -2550;
   _hum = 0;
}

/**
 * @brief helper function to print byte as bit
 * 
 * @param value byte with 8 bits
 */
void AM2302_Tools::print_byte_as_bit(char value) {
   for (int i = 7; i >= 0; --i) {
      char c = (value & (1 << i)) ? '1' : '0';
      printf("%c", c);
   }
   printf("\n");
}