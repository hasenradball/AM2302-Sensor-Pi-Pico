/**
 * @file AM2302-Sensor-Pi-Pico.h
 * @author Frank Häfele (mail@frankhaefele.de)
 * @brief Measure Sensor Data of AM2302-Sensor with Pi Pico
 * @version 1.0
 * @date 2026-03-02
 * 
 * @copyright Copyright (c) 2026 Frank Häfele
 * 
 */

#ifndef __AM2302_SENSOR_PI_PICO_H__
#define __AM2302_SENSOR_PI_PICO_H__
#include <cstdint>
#include "hardware/pio.h"

namespace AM2302 {

   constexpr const char * AM2302_STATE_OK{"OK"};
   constexpr const char * AM2302_STATE_UNKNOWN{"State Unknown!"};
   constexpr const char * AM2302_STATE_ERR_CKSUM{"Error: Checksum"};
   constexpr const char * AM2302_STATE_ERR_TIMEOUT{"Error: Timeout"};
   constexpr const char * AM2302_STATE_ERR_READ_FREQ{"Error: Read Frequency"};

   constexpr int8_t AM2302_READ_OK          {0};
   constexpr int8_t AM2302_ERROR_CHECKSUM   {-1};
   constexpr int8_t AM2302_ERROR_TIMEOUT    {-2};
   constexpr int8_t AM2302_ERROR_READ_FREQ  {-3};

   // define timeout in µs
   constexpr uint16_t READ_TIMEOUT        {2000U};
   // cycle time in ms
   constexpr uint16_t READ_FREQUENCY      {2000U};

   class AM2302_Sensor {

      public:
         explicit AM2302_Sensor(uint8_t pin);
         bool begin();
         int8_t read();
         float get_Temperature() const {return _temp * 0.1F;}
         float get_Humidity() const {return _hum * 0.1F;}
         const char * get_sensorState(int8_t state) const;

      private:
         unsigned long _millis_last_read;
         PIO _pio {pio0};
         int _sm;
         uint16_t _hum {0};
         int16_t _temp {0};
         uint8_t _pin;
         bool _checksum_ok {false};
         
         static unsigned int _offset;

         int8_t read_sensor();
         void resetData();
   };
}

namespace AM2302_Tools {
    void print_byte_as_bit(char value);
}

#endif
