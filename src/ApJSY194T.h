#ifndef ApJSY194T_h
#define ApJSY194T_h

/*
  ApJSY194T.h - ApJSY194T include file
  For instructions, go to https://github.com/AntonioPrevitali/ApJSY194T
  Created by Antonio Previtali, 07/07/2025.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the Gnu general public license version 3
*/

#include "Arduino.h"

#ifdef __AVR__
#include <SoftwareSerial.h>
#endif

class ApJSY194T {
  public:
    ApJSY194T(void);
    void BeginSerial(HardwareSerial& xserial);
    #ifdef __AVR__
    void BeginSerial(SoftwareSerial& xserial);
    #endif

    boolean okReadMeter(void);

    float   Voltage1;    // First channel Voltage
    float   Current1;    // First channel current
    float   Power1;      // First channel active power, negativa se potenza inviata in rete.
    float   Import1;     // energia kwh totali comprati da rete. Kwh  Positive active energy of first channel
    float   Export1;     // energia kwh totali inviati in rete.  kwh  Negative active energy of first channel
    float   PowFact1;    // First channel power factor
    
    float   Frequenz;    // Frequenza

    float   Voltage2;    // First channel Voltage
    float   Current2;    // First channel current
    float   Power2;      // First channel active power, negativa se potenza inviata in rete.
    float   Import2;     // energia kwh totali comprati da rete. Kwh  Positive active energy of first channel
    float   Export2;     // energia kwh totali inviati in rete.  kwh  Negative active energy of first channel
    float   PowFact2;    // First channel power factor
 

  private:
    boolean  waitResponse(void);
    uint16_t calculateCrc(byte* data, size_t size);
    uint32_t bytesToUInt32(byte* regData);
    boolean  okTranza(void);

    Stream *_serial;

    uint8_t _buffer[61];

};


#endif
