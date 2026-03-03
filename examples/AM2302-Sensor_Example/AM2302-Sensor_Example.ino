/**
 * @file AM2302-Sensor_Example.ino
 * @author Frank Häfele (mail@frankhaefele.de)
 * @brief Example how to use the library
 * @version 1.0
 * @date 2026-03-03
 * 
 * @copyright Copyright (c) 2026 Frank Häfele
 * 
 */

#include <AM2302-Sensor-Pi-Pico.h>

constexpr unsigned int SENSOR_PIN {7U};

AM2302::AM2302_Sensor am2302{SENSOR_PIN};

void setup() {
   Serial.begin(115200);
   while (!Serial) {
      yield();
   }
   Serial.print(F("\n >>> AM2302-Sensor_Example <<<\n\n"));

   // set pin and check for sensor
   if (am2302.begin()) {
      // this delay is needed to receive valid data,
      // when the loop directly read again
      delay(3000);
   }
   else {
      while (true) {
      Serial.println("Error: sensor check. => Please check sensor connection!");
      delay(10000);
      }
   }
}

void loop() {
   // put your main code here, to run repeatedly:
   auto status = am2302.read();
   Serial.print("\n\nstatus of sensor read(): ");
   Serial.println(status);

   Serial.print("Temperature: ");
   Serial.println(am2302.get_Temperature());

   Serial.print("Humidity:    ");
   Serial.println(am2302.get_Humidity());
   delay(5000);
}