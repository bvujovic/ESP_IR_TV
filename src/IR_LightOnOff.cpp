// Ovde je kôd za aparat koji se nalazi u plafonjerci. MCU je Arduino Pro Mini.
// Paljenje/gasenje svetla u dnevnoj sobi preko TV daljinskog.
// Zuto dugme   - relej, tj. obe sijalice
// Zeleno dugme - on/off LED diode

enum State
{
  Off,     // sva svetla su ugasena
  LedOn,   // srednje svetlo (LED traka) je upaljeno
  LightsOn // sijalice su upaljene
};
State state = Off;

#include <IRremote.h> //* Disable (set to 0) all the protocols you do not need/want!
const int pinIR = 4;
IRrecv irrecv(pinIR);
decode_results results;
#include "NecCodes.h"

const int pinLED = 3;
const int pinRelay = 2; // za relej koji koristim vazi: HIGH(true) -> OFF, LOW(false) -> ON

const int itvMain = 200;

void translateIR()
{
  //T Serial.println(results.value, HEX);
  if (results.value == NecCode::ColorGreen)
    state = (state == LedOn) ? Off : LedOn;
  else if (results.value == NecCode::ColorYellow)
    state = (state == LightsOn) ? Off : LightsOn;

  if (results.value == NecCode::MediumLightOn)
    state = State::LedOn;
  if (results.value == NecCode::HighLightOn)
    state = State::LightsOn;
  if (results.value == NecCode::LightOff)
    state = State::Off;

  digitalWrite(pinRelay, (state != LightsOn));
  digitalWrite(pinLED, state == LedOn);

  delay(itvMain);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, true);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, false);

  irrecv.enableIRIn();
  //T Serial.begin(9600);
}
void loop()
{
  if (irrecv.decode(&results))
  {
    translateIR();
    irrecv.resume();
  }
}
