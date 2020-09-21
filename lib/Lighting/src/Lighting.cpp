#include "Lighting.h"

void Lighting::getCurrentTime()
{
    //T Serial.print("Call Lighting::getCurrentTime(): ");
    bool isTimeSet = false;
    for (int i = 0; i < 10 && !isTimeSet; i++)
    {
        isTimeSet = ntp.setSNTPtime();
        //T Serial.print('*');
    }
    //T Serial.println(isTimeSet ? " success" : " FAIL!!");
    msTimeSet = millis();
}

void Lighting::init(bool on, int startHour, int startMin, int endHour, int endMin, int devMin)
{
    if (!on)
        return;
    getCurrentTime();
    randomSeed(analogRead(0));
    this->isSystemOn = on;
    this->isLightOn = false;
    this->startMin = startHour * 60 + startMin;
    this->endMin = endHour * 60 + endMin;
    this->devMin = devMin;
    this->startMinAct = this->startMin + random(-devMin, devMin);
    //T Serial.println(startMinAct);
}

int Lighting::refresh()
{
    if (!isSystemOn)
        return 0;

    StrDateTime now = ntp.getTime(1.0, 1);
    int min = now.hour * 60 + now.minute;
    // ako je 8AM - uzeti tekuce date-time
    if (min == 8 * 60 && millis() - msTimeSet > 60 * 1000)
        getCurrentTime();

    // da li je vreme za paljenje ili gasenje svetla
    if (!isLightOn && min == this->startMinAct)
    {
        this->endMinAct = this->endMin + random(-devMin, devMin);
        //T Serial.println(endMinAct);
        isLightOn = true;
        return 1;
    }
    if (isLightOn && min == this->endMinAct)
    {
        this->startMinAct = this->startMin + random(-devMin, devMin);
        //T Serial.println(startMinAct);
        isLightOn = false;
        return -1;
    }
    return 0;
}