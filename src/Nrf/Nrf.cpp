#include "Arduino.h"
#include "Nrf.h"

Nrf::Nrf(RF24 *rfradio)
{
    radio = rfradio;
}

void Nrf::readMessage(void* message)
{
    if (radio->available())
    {
        while (radio->available())
        {
            radio->read(message, radio->getDynamicPayloadSize());
        }
    }
}

bool Nrf::sendMessage(const void *message, uint8_t len)
{
    radio->stopListening();
    bool ok = radio->write(message, len);
    radio->startListening();

    return ok;
}

bool Nrf::waitForResponse(int wait)
{
    unsigned long started_waiting_at = millis();
    bool timeout = false;

    while (!radio->available() && !timeout)
    {
        if (millis() - started_waiting_at > wait)
        {
            timeout = true;
        }
    }

    return !timeout;
}