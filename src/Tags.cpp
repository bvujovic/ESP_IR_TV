#include "Tags.h"
#include "FS.h"

Tags::Tags(/* args */)
{
}

String Tags::ReadTagsFile()
{
  // http://arduino.esp8266.com/Arduino/versions/2.0.0/doc/filesystem.html#file-object
  // https://www.youtube.com/watch?v=1coe2576Rcw

  //B SPIFFS.begin();
  File fp = SPIFFS.open("/dat/tags.csv", "r");
  // Citanje mozda moze da se uradi uz pomoc readString() ili readStringUntil('\n') ??
  String s = "";
  if (fp)
  {
    while (fp.available()) {
      s.concat((char)fp.read());
    }
    fp.close();
  }
  else
    Serial.println("tags.csv open (r) faaail.");
  return s;
}

void Tags::WriteTagsFile(String s)
{
  SPIFFS.begin();
  File fp = SPIFFS.open("/dat/tags.csv", "w");
  if (fp)
  {
    fp.print(s);
    // fp.println("1|fav|1,123,111");
    // fp.println("2|sport|");
    // fp.println("3|ubistva|216,217");
    fp.close();
  }
  else
    Serial.println("tags.csv open (w) faaail.");
}
