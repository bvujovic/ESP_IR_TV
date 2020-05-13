#include "Channel.h"

uint Channel::idxSelectedGen = 1;

void Channel::Init(String name, short number, bool spin)
{
    this->name = name;
    this->number = number;
    this->spin = spin;
}

String Channel::ToString()
{
    String s = name;
    s.concat(SepProps);
    s.concat(number);
    s.concat(SepProps);
    s.concat(spin);
    s.concat(SepProps);
    s.concat(idxSelected);
    if (tags != "") // ako kanal ima tagove - dodaj ih u string
    {
        s.concat(SepProps);
        s.concat(tags);
    }
    return s;
}

void Channel::ToggleSelected()
{
    idxSelected = IsSelected() ? 0 : idxSelectedGen++;
}

String Channel::ChannelsToString(Channel *channels, uint cntChannels)
{
    String s = "";
    for (uint i = 0; i < cntChannels; i++)
    {
        s.concat(channels[i].ToString());
        s.concat('\n');
    }
    return s;
}
