#pragma once

#include <Arduino.h>
#include <SNTPtime.h>

// Paljenje/gasenje svetla svake veceri. Trip da izgleda kao da je neko u stanu a otisao je na more 👻
class Lighting
{
private:
    // Da li je automatsko paljenje/gasenje svetla u zadato vreme ukljuceno ili ne.
    bool isSystemOn = false;
    // Da li je svetlo trenutno upaljeno ili ne.
    bool isLightOn = false;
    // Nivo/jacina svetla. 2 - srednje, 3 - jako
    int lightLevel;
    // U kom minutu u danu se svetlo pali. Primer: 20:30 -> 20*60 + 30
    int startMin;
    // U kom minutu u danu se svetlo gase. Primer: 01:15 -> 1*60 + 15
    int endMin;
    // Maksimalna devijacija u odnosu na standardno vreme. Poenta je da se svetlo ne pali/gasi uvek tacno u minut.
    int devMin;
    // Kada ce se sledeci put zaista upaliti svetlo.
    int startMinAct;
    // Kada ce se sledeci put zaista ugasiti svetlo.
    int endMinAct;

    // Objekat neophodan za rad sa tacnim vremenom.
    SNTPtime ntp;
    // Vreme (u ms) kada je poslednji put uzeto tacno vreme.
    ulong msTimeSet;
    // Uzimanje tacnog vremena.
    void getCurrentTime();

public:
    // Inicijalizacija objekta vrednostima: da li automatski ON-OFF svetla aktivan, kada se svetlo pali, a kada gasi.
    // Samo ova metoda moze da promeni: isSystemOn, startMin, endMin, devMin.
    void init(bool on, int level, int startHour, int startMin, int endHour, int endMin, int devMin);
    // 1 treba upaliti svetlo, -1 treba ugasiti svetlo, 0 nista ne ciniti
    int refresh();
    // Nivo/jacina svetla. 2 - srednje, 3 - jako
    int getLightLevel() { return lightLevel; }
};
