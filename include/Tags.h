#ifndef TAGS_H
#define TAGS_H

#include "Arduino.h"

class Tags
{
private:
    /* data */
public:
    Tags();
    static String ReadTagsFile();
    static void WriteTagsFile(String s);
};

#endif