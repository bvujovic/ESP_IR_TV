#pragma once

#include <Arduino.h>

struct Channel
{
    static const char SepProps = '|'; // separator za razdvajanje svojstava u objektu

    static uint idxSelectedGen;

    String name;      // Naziv kanala.
    short number;     // Broj kanala na TVu.
    bool spin;        // DEPRECATED. false(0) ako je prvi od dva kanala na istom broju, true(1) je drugi.
    uint idxSelected; // Broj kanala u selected listi. 0 - nije selected.
    String tags;      // Lista tagova koji se odnose na ovaj kanal.

    Channel() {}
    // Inicijalizacija Channel objekta prosledjenim podacima.
    void Init(String name, short number, bool spin = false);
    // Tekstualna serijalizacija podataka o kanalu. Npr "0|N1 HD|1|1"
    String ToString();
    // Da li je kanal selektovan, tj. da li pripada listi kanala kroz koje trenutno korisnik prolazi.
    bool IsSelected() { return idxSelected != 0; }
    // Promena selektovanosti kanala. Nije selektovan <-> selektovan.
    void ToggleSelected();
    // Pomeranje kanala jedno mesto nagore u listi selektovanih.
    static void ChannelSelMoveUp(Channel *channels, uint cntChannels, uint idxChan);
    // Tekstualna serijalizacija podataka o svim kanalima iz prosledjenog niza.
    static String ChannelsToString(Channel *channels, uint cntChannels);
};