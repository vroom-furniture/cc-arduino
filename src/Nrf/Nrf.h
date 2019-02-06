#ifndef Nrf_h
#define Nrf_h

#include "Arduino.h"
#include <nRF24L01.h>
#include <RF24.h>

class Nrf
{
  public:
    Nrf(RF24 *rfradio);
    RF24 *radio;

    void readMessage(char* message);
    bool sendMessage(const void *message, uint8_t len);
    bool waitForResponse(int wait);
};

#endif