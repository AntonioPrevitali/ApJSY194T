#include "ApJSY194T.h"

/*
  ApJSY194T.h - ApJSY194T Implementation file
  For instructions, go to https://github.com/AntonioPrevitali/ApJSY194T
  Created by Antonio Previtali, 07/07/2025.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the Gnu general public license version 3
*/

ApJSY194T::ApJSY194T(void)
{
 Voltage1 = 0;
 Current1 = 0;
 Power1 = 0;
 Import1 = 0;
 Export1 = 0;
 PowFact1 = 0;
 Frequenz = 0;
 Voltage2 = 0;
 Current2 = 0;
 Power2 = 0;
 Import2 = 0;
 Export2 = 0;
 PowFact2 = 0;
}


void ApJSY194T::BeginSerial(HardwareSerial& xserial)
{
   xserial.begin(19200,SERIAL_8N1);
   _serial = &xserial;
}

#ifdef __AVR__
void ApJSY194T::BeginSerial(SoftwareSerial& xserial)
{
    xserial.begin(19200);   // x softwareserial farà 8N1 che comunque funziona...
   _serial = &xserial;
}
#endif

uint16_t ApJSY194T::calculateCrc(byte* data, size_t size)
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


uint32_t ApJSY194T::bytesToUInt32(byte* regData)
{
    uint32_t result = 0;
    for (int8_t i = 3; i >= 0; i--) {
        ((uint8_t*)&result)[i] = regData[3 - i];
    }
    return result;
}




boolean ApJSY194T::okTranza(void)
{
  uint8_t xreq[] = {0x01, 0x03, 0x00, 0x48, 0x00, 0x0E, 0x44, 0x18};  // legge tutti e 14 i registri.
  while (_serial->available()) _buffer[0] = _serial->read(); // pulisce eventuali skifezze nel buffer rx.
  _serial->write(xreq, 8);
  _serial->flush();
  if (!waitResponse()) return false;   // la risposta va tutta nel buffer e sono ben 61 caratteri
                                       // conviene fare cosi, fare letture di registri mirate (esempio power e direzione)
                                       // alla fine si impiega piu tempo... perche per ogni interrogazione impiega circa 30ms prima
                                       // di iniziare a rispondere... JSX lavora cosi, meglio leggere tutti i 61 byte...
  return true;
}



boolean ApJSY194T::okReadMeter(void)
{
  byte*   pDat;   // un pointer sul buffer.
  uint8_t xdir1;
  uint8_t xdir2;

  if (!okTranza()) return false;
  // conversione in float dei valori (dura circa 300 microsecondi le istruzioni sequenti)
  pDat = _buffer+3;
  Voltage1 = bytesToUInt32(pDat);
  Voltage1 = Voltage1 / 10000;
  pDat += 4;
  Current1 = bytesToUInt32(pDat);
  Current1 = Current1 / 10000;
  pDat += 4;
  Power1 = bytesToUInt32(pDat);
  Power1 = Power1 / 10000;
  pDat += 4;
  Import1 = bytesToUInt32(pDat);
  Import1 = Import1 / 10000;
  pDat += 4;
  PowFact1 = bytesToUInt32(pDat);
  PowFact1 = PowFact1 / 1000;       // qui Mille.
  pDat += 4;
  Export1 = bytesToUInt32(pDat);
  Export1 = Export1 / 10000;
  // i due campi con la direction della potenza.
  pDat += 4;
  xdir1 = *pDat;
  pDat += 1;
  xdir2 = *pDat;
  pDat += 3;
  // campo frequenza
  Frequenz = bytesToUInt32(pDat);
  Frequenz = Frequenz / 100;       // qui 100 
  pDat += 4;  
  // idem per il canale 2
  Voltage2 = bytesToUInt32(pDat);
  Voltage2 = Voltage2 / 10000;
  pDat += 4;
  Current2 = bytesToUInt32(pDat);
  Current2 = Current2 / 10000;
  pDat += 4;
  Power2 = bytesToUInt32(pDat);
  Power2 = Power2 / 10000;
  pDat += 4;
  Import2 = bytesToUInt32(pDat);
  Import2 = Import2 / 10000;
  pDat += 4;
  PowFact2 = bytesToUInt32(pDat);
  PowFact2 = PowFact2 / 1000;       // qui Mille.
  pDat += 4;
  Export2 = bytesToUInt32(pDat);
  Export2 = Export2 / 10000;
  // ed ora il segno per la potenza.
  if (xdir1 == 1) Power1 = - Power1;
  if (xdir2 == 1) Power2 = - Power2; 
  return true;
}




boolean ApJSY194T::waitResponse(void)
{
  unsigned long startTimeMs = millis();
  unsigned long startTimeus;
  uint16_t ilcrc;
  uint8_t numBytes = 0;
  while (!_serial->available()) {
    if (millis() - startTimeMs >= 100) {  // di solito servono circa 30ms prima che arrivi il primo carattere risposta..
      return false;                       // a sicurezza metto 100 ms che vedi anche sotto...
    }
  }
  do {
    do {
      if (_serial->available()) {
        startTimeus = micros();
        _buffer[numBytes] = _serial->read();
        numBytes++;
      }
    } while (micros() - startTimeus <= 2870 && numBytes < 61);   // sarebbe 1435 tempo massimo tra un carattere e l'altro i caratteri devono arrivare uno di seguito all'altro con un certo ritmo imposto dal modbus
                                                                 // a causa degli interrupt questo controllo è critico e discutibile
                                                                 // inoltre su questi moduli questo controllo sembra superfuo cosi lascio il 2870 che
                                                                 // derivava dal modbus a 9600 bps.
    if (numBytes < 61) // cioè uscito dal do per fuori ritmo..
    {
      numBytes = 0;   // caratteri arrivati prima sono rumore o skifezze li butta via !
    }
  } while (millis() - startTimeMs <= 100 && numBytes < 61);  // di solito servono 64 ms per avere tutta la risposta. prudenzialmente metto 100ms
  if (numBytes < 61)
  {
    // non sono arrivati tutti i caratteri della risposta quindi non va bene ritorna false.
    return false;
  }
  // ok vediamo se il crc è giusto
  ilcrc = calculateCrc(_buffer,59);   // circa 400 microsecondi per calcolare questo crc
  if (_buffer[59] != highByte(ilcrc) || _buffer[60] != lowByte(ilcrc) )
  {
     return false;  // crc errato !
  }
  // vediamo anche se questi sono giusti !
  if (_buffer[0] != 1 || _buffer[1] != 3 || _buffer[2] != 56) return false;
  return true; // ok è giusto !
}
