// Testiranje prijema IR signala.
// Ugradjena LED dioda na Arduinu se pali/gasi kada dobije signal sa crvenog tastera.

#include <IRremote.h>
const int pinIR = 4;
IRrecv irrecv(pinIR);
decode_results results;
#include "NecCodes.h"

const int pinLed = LED_BUILTIN;
bool isLedOn = false;

const int itvMain = 200;

void translateIR()
{
  if (results.value == NecCode::ColorRed)
    isLedOn = !isLedOn;

  digitalWrite(pinLed, isLedOn);

  delay(itvMain);
}

void setup()
{
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, isLedOn);

  irrecv.enableIRIn();
}

void loop()
{
  if (irrecv.decode(&results))
  {
    translateIR();
    irrecv.resume();
  }
}
