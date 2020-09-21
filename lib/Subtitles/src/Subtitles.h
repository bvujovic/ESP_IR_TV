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

//* Dorade. Nove metode:
//      cancel()    Odustajanje od subs-a za svaku promenu kanala u 5sec intervalu od prebacivanja na 217 i sl
//      refresh()   Redovno pozivanje u loop-u. Metoda ispituje da li treba da se ispali subs-subs-ok