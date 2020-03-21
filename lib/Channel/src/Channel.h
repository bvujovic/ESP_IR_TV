#pragma once

#include <Arduino.h>

struct Channel
{
    static const char SepProps = '|'; // separator za razdvajanje svojstava u objektu

    String name;  // naziv kanala
    short number; // broj kanala na TVu
    bool spin;    // false(0) ako je prvi od dva kanala na istom broju, true(1) je drugi
    bool isSelected;
    String tags;

    Channel() {}

    void init(String name, short number, bool spin = false)
    {
        this->name = name;
        this->number = number;
        this->spin = spin;
    }

    String toString()
    {
        String s = name;
        s.concat(SepProps);
        s.concat(number);
        s.concat(SepProps);
        s.concat(spin);
        s.concat(SepProps);
        s.concat(isSelected);
        if (tags != "") // ako kanal ima tagove - dodaj ih u string
        {
            s.concat(SepProps);
            s.concat(tags);
        }
        return s;
    }
};