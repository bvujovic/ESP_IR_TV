#pragma once

#include <Arduino.h>
#include <LinkedList.h>

// Objekat ove klase prihvata listu kanala (String) i proverava da li se kanal (int) nalazi u toj listi.
class Subtitles
{
private:
    LinkedList<int> channels = LinkedList<int>();

public:
    // Parsira se string formata "217,111" i na osnovu njega se pravi lista (int) kanala sa titlovima
    void parseChannels(String s);
    // true - ako se c nalazi u listi kanala za koje treba prikazati titlove
    bool forChannel(int c);
};
