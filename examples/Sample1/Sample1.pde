#include <Arduino.h>

// Code for Arduino Mega2560 use only once to change the speed from 4800 bps default of the JSY-MK-194T module to 19200 bps

// uses the Serial1 of the Mega2560 to communicate with the module
// and Serial for serial monitor, console.


uint16_t calculateCrc(byte* data, size_t size)
{
    uint16_t _crc;
    _crc = 0xFFFF;
    for (size_t i = 0; i < size; i++) {
        _crc ^= data[i];
        for (byte j = 0; j < 8; j++) {
            if (_crc & 0x0001)
                _crc = (_crc >> 1) ^ 0xA001; //< fixed polynomial
            else
                _crc = _crc >> 1;
        }
    }
    return _crc >> 8 | _crc << 8;
}



void setup() {
 
  Serial.begin(9600);  // console monitor seriale.

  Serial.println("Set Serial1 at 4800 bps default of JSY-MK-194T");
    
  Serial1.begin(4800,SERIAL_8N1);     // if already set to 19200 try Serial1.begin(19200,SERIAL_8N1);
 
  delay(200);  // a bit of time used by jsy-mk-194t serial to start (NOT REMOVE, TEST)

  Serial.println("Send command to change speed of JSY-MK-194T at 19200 bps");

  uint8_t xreq[] = {0x00, 0x10, 0x00, 0x04, 0x00, 0x01, 0x02, 0x01, 0x07 , 0x0, 0x0 }; // setting to 19200 bps.
  //                  |
  //                  |-> note the 00 in the address ID ... (ok id 0 because the command is used not only to change speed but also to change ID)
  // response if command executed successfully
  //                                            0x1,0x10,0x0,0x4,0x0,0x1, 0x40,0x8

  
  uint16_t ilcrc;
  uint8_t  xbyte;
  
  ilcrc = calculateCrc(xreq,sizeof(xreq)-2);
  xreq[sizeof(xreq)-2] =  highByte(ilcrc);
  xreq[sizeof(xreq)-1] = lowByte(ilcrc);

  while (Serial1.available()) xbyte = Serial1.read();
  Serial1.write(xreq, sizeof(xreq));
  Serial1.flush();

  Serial.println("The meter should respond with 1 10 0 4 0 1 40 8");
  Serial.println("the meter response is:");

}



void loop() {
    uint8_t  xbyte;
  
    if (Serial1.available()) {

        xbyte = Serial1.read();
        Serial.println(xbyte,HEX);

    }

}
