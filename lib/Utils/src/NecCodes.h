#pragma once

// Kodovi za NEC daljinski upravljac.
// Za listu svih kododva za daljinac pogledati fajl: data/dat/lg_ir_codes.ini
enum NecCode
{
    OnOff = 0x20DF10EF,
    Mute = 0x20DF906F,
    Back = 0x20DF14EB,
    Guide = 0x20DFD52A,
    VolUp = 0x20DF40BF,
    VolDown = 0x20DFC03F,

    Subtitle = 0x20DF9C63,
    Settings = 0x20DFC23D,
    Exit = 0x20DFDA25,
    Inputs = 0x20DFD02F,
    Up = 0x20DF02FD,
    Down = 0x20DF827D,
    Left = 0x20DFE01F,
    Ok = 0x20DF22DD,
    ArrowRight = 0x20DF609F,

    ColorGreen = 0x20DF8E71,
    MediumLightOn = 0x20DF8E17,
    ColorYellow = 0x20DFC639,
    HighLightOn = 0x20DFC693,
    LightOff = 0x20DFC6FB,
    ColorRed = 0x20DF4EB1,
    ColorBlue = 0x20DF8679,
};
