#include "Subtitles.h"

void Subtitles::parseChannels(String s)
{
    // EG s="217,111", s="217, 111;123"
    char strNum[5]; // char* bafer za tekuci broj tj. kanal
    unsigned int j = 0;     // brojac za strNum
    channels.clear();
    for (unsigned int i = 0; i < s.length(); i++)
    {
        if (isdigit(s[i]))
            strNum[j++] = s[i];
        else if (j != 0)
        {
            strNum[j] = '\0';
            j = 0;
            channels.add(atoi(strNum));
        }
    }
    if (j != 0)
        channels.add(atoi(strNum));
}

bool Subtitles::forChannel(int c)
{
    int n = channels.size();
    for (int i = 0; i < n; i++)
        if (channels.get(i) == c)
            return true;
    return false;
}