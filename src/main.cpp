#include <Arduino.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

#include <ArduinoOTA.h>
bool isOtaOn = false; // da li je OTA update u toku

#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(D2);
#include <NecCodes.h>

#include <EasyINI.h>
EasyINI ei("/dat/config.ini");
#include "Subtitles.h"
Subtitles subs;
#include "Lighting.h"
Lighting lighting;
// #include <EasyFS.h>

#include "UpdateCSV.h"
#include "Channel.h"
#include <LittleFsUtils.h>

typedef unsigned int uint;

const uint pinLed = LED_BUILTIN;
const uint itvShortDelay = 50;      // pauza izmedju 2 uzastopna signala
const uint itvLongDelay = 200;      // dodatna pauza izmedju 2 uzastopna signala kada su susedne cifre iste
const uint itvSubtitles = 5000;     // vreme (u ms) koje treba da prodje od promene kanala do aktiviranja Subtitles
uint cntChannels = 0;               // broj TV kanala koji se pamte u nizu channels
bool guideLastAct = false;          // da li je poslednja akcija klik na GUIDE
uint msSubtitles = 0;               // vreme aktiviranja Subtitle opcije. 0 - ne traze se titlovi za kanal
const char *CH_PREFIX = "ch";       // komanda za promenu kanala ima ovaj prefiks
const char *SEL_CH_SUFFIX = "_sel"; // komanda koja stavlja kanal u selektovane ima ovaj sufiks

Channel *channels;

void initChannels()
{
  // TODO zapamtiti koji su kanali bili selektovani (idxSelected>0) i kasnije ih ponovo postaviti u nizu
  if (cntChannels > 0) // ako se ova funkcija zove drugi, treci put... i kanali su vec popunjeni
    free(channels);

  File fp = LittleFS.open("/dat/channels.csv", "r");
  cntChannels = 0;

  int b; // bajt koji je procitan iz fajla
  if (fp)
  {
    // brojanje kanala (redova) u .csv fajlu
    while (fp.available())
    {
      b = fp.read();
      if (b == '\n')
        cntChannels++;
    }
    cntChannels++; // poslednji red nema prelazak u novi red
    channels = (Channel *)calloc(cntChannels, sizeof(Channel));

    // ucitavanje kanala: broj i naziv
    int i = 0;
    fp.seek(0, SeekSet);
    while (fp.available())
    {
      String l = fp.readStringUntil('\n');
      l.trim();
      int idx = l.indexOf(Channel::SepProps);
      if (idx == -1)
        break;
      String strNumber = l.substring(0, idx);
      String name = l.substring(idx + 1);
      channels[i].Init(name, strNumber.toInt());
      i++;
    }
    fp.close();
  }
  else
    Serial.println("channels.csv open (r) faaail.");
}

void handleGetChannels()
{
  server.send(200, "text/x-csv", Channel::ChannelsToString(channels, cntChannels));
}

const String TAGS_CSV = "/dat/tags.csv";

void handleGetTags()
{
  server.send(200, "text/x-csv", LittleFsUtils::ReadFile(TAGS_CSV));
}

void handleSetTags()
{
  LittleFsUtils::WriteFile(TAGS_CSV, server.arg("plain"));
  SendEmptyText(server);
}

void handleChanSelMoveUp()
{
  String idxChan = server.arg("idxChan");
  Channel::ChannelSelMoveUp(channels, cntChannels, idxChan.toInt());
  SendEmptyText(server);
}

uint msTurnLater;       // Vreme kada treba prebaciti na kanal chTurnLater.
short chTurnLater = -1; // Broj kanala na koji treba kasnije prebaciti.

// Prebacivanje na zadati kanal za x minuta.
void handleTurnLater()
{
  float min = server.arg("min").toFloat();
  if (min == 0)
    chTurnLater = -1;
  else
  {
    msTurnLater = millis() + min * 60 * 1000;
    chTurnLater = server.arg("ch").toInt();
  }
  SendEmptyText(server);
}

// test json
void handleTest()
{
  Serial.println("/test");
  server.send(200, "application/json", "{ 'app': 'ESP_IR_TV' }");
}

// Slanje IR kodova ka TVu.
void sendIRcodes(long *codes)
{
  for (short i = 0; codes[i] != 0; i++)
  {
    if (i != 0 && codes[i] == codes[i - 1])
      delay(itvLongDelay);
    // T Serial.println(codes[i]);
    irsend.sendNEC(codes[i]);
    delay(itvShortDelay);
  }
}

// Vraca niz kodova potrebnih za prebacivanje na dati kanal (chNumber) sa datim spinom.
// Poslednji kôd ce imati vrednost 0.
long *irCodes(short chNumber, bool spin)
{
  long codes[10];
  codes[0] = 0x20DF08F7;
  codes[1] = 0x20DF8877;
  codes[2] = 0x20DF48B7;
  codes[3] = 0x20DFC837;
  codes[4] = 0x20DF28D7;
  codes[5] = 0x20DFA857;
  codes[6] = 0x20DF6897;
  codes[7] = 0x20DFE817;
  codes[8] = 0x20DF18E7;
  codes[9] = 0x20DF9867;
  long codeArrowDown = 0x20DF827D;
  long codeOK = 0x20DF22DD;

  String s = String(chNumber);
  byte len = s.length() + 2 /*kôd 0 za kraj niza i OK dugme*/;
  if (spin)
    len += 1; // strelica na dole

  long *a = new long[len];
  for (short i = len - 1; i >= 0; i--)
    a[i] = codes[s.charAt(i) - '0'];
  if (spin)
    a[len - 3] = codeArrowDown;
  a[len - 2] = codeOK;
  a[len - 1] = 0;
  return a;
}

short chNumPrev = -1; // prethodni broj kanala
short chNumCurr = -1; // tekuci broj kanala

// Pamcenje tekuceg kanala kao prethodnog i novog kao tekuceg.
void pushToPrev(short chNum)
{
  if (chNum != chNumCurr)
  {
    chNumPrev = chNumCurr;
    chNumCurr = chNum;
  }
}

// Prebacivanje na kanal sa datim brojem.
void turnTo(short chNum)
{
  pushToPrev(chNum);
  sendIRcodes(irCodes(chNum, false));
  if (subs.forChannel(chNum))
    msSubtitles = millis();
}

// Akcija daljinskog (act)
void handleAction()
{
  String cmd = server.arg("cmd");
  // T EasyFS::addf(cmd);

  // sve komande osim ovde izuzetih znace otkazivanje Subtitles opcije
  if (cmd != "mute" && !cmd.startsWith("vol") && !cmd.startsWith("color") && !cmd.startsWith("back") && cmd.indexOf("light") == -1 && !cmd.endsWith(SEL_CH_SUFFIX))
    msSubtitles = 0;

  // gornji deo daljinskog
  if (cmd == "onOff")
    irsend.sendNEC(OnOff);
  if (cmd == "mute")
    irsend.sendNEC(Mute);
  if (cmd == "volUp")
  {
    irsend.sendNEC(VolUp);
    delay(itvLongDelay);
    irsend.sendNEC(VolUp);
  }
  if (cmd == "volDown")
  {
    irsend.sendNEC(VolDown);
    delay(itvLongDelay);
    irsend.sendNEC(VolDown);
  }
  if (cmd == "colorRed") // ON/OFF Bunny Alarm (poseban aparat/projekat)
    irsend.sendNEC(ColorRed);
  if (cmd == "colorGreen") // ON/OFF srednjeg svetla
    irsend.sendNEC(ColorGreen);
  if (cmd == "colorYellow") // ON/OFF jakog svetla
    irsend.sendNEC(ColorYellow);
  // posebne komande za paljenje i gasenje svetla (nema na daljincu)
  if (cmd == "mediumLightOn") // ON srednjeg svetla
    irsend.sendNEC(MediumLightOn);
  if (cmd == "highLightOn") // ON jakog svetla
    irsend.sendNEC(HighLightOn);
  if (cmd == "lightOff") // OFF svetla
    irsend.sendNEC(LightOff);

  if (cmd == "guide")
  {
    irsend.sendNEC(Guide);
    guideLastAct = true;
  }
  if (cmd == "arrRight")
    irsend.sendNEC(ArrowRight);
  if (cmd == "back")
    irsend.sendNEC(Back);
  if (cmd == "back2" && chNumPrev != -1)
    turnTo(chNumPrev);

  if (cmd.startsWith(CH_PREFIX)) // kanali
  {
    String ch = cmd.substring(2);
    if (cmd.endsWith(SEL_CH_SUFFIX)) // selektovanje kanala
    {
      ch = ch.substring(0, ch.lastIndexOf(SEL_CH_SUFFIX));
      int idx = ch.toInt();
      channels[idx].ToggleSelected();
    }
    else // aktiviranje kanala
    {
      if (guideLastAct)
      {
        guideLastAct = false;
        irsend.sendNEC(Back);
        delay(itvLongDelay);
      }
      int idx = ch.toInt();
      // T Serial.println(channels[idx].ToString());
      turnTo(channels[idx].number);
    }
  }

  SendEmptyText(server);
}

void loadConfigIni()
{
  ei.open(FMOD_READ);
  // subtitles
  subs.parseChannels(ei.getString("subtitles"));
  // lighing
  lighting.init(ei.getInt("absentLightOn"), ei.getInt("absentLightLevel"),
                ei.getInt("absentLightStartHour"), ei.getInt("absentLightStartMin"),
                ei.getInt("absentLightEndHour"), ei.getInt("absentLightEndMin"), ei.getInt("absentLightTimeDev"));
  ei.close();
}

String textFileName;

void handleLoadTextFile()
{
  textFileName = server.arg("name");
  File f = LittleFS.open(textFileName, "r");
  if (f)
  {
    server.streamFile(f, "text/css");
    f.close();
  }
  else
    Serial.println(textFileName + " - error reading file.");
}

void handleSaveTextFile()
{
  LittleFsUtils::WriteFile(textFileName, server.arg("plain"));
  loadConfigIni();
  initChannels();
  SendEmptyText(server);
}

void setup()
{
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, false);

  // EasyFS::setFileName("/dat/msgs.log"); // http://192.168.0.20/loadTextFile?name=dat/msgs.log

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  //* Cekanje na internet u spavanju - ovo ne radi na nekim ESP8266 modulima
  // if (!ConnectToWiFi())
  //   ESP.deepSleep(5 * 60 * 1000000); // cekanje/spavanje 5min da ruter ozivi posle dolaska struje npr.
  //* Cekanje na internet bez spavanja - ovo radi na svim ESP8266 modulima
  while (!ConnectToWiFi())
    delay(1 * 60 * 1000); // cekanje 1min

  UpdateCSV::DownloadNewCsvIN();
  UpdateCSV::UploadNewCsvIN();
  loadConfigIni();
  SetupIPAddress(20);
  server.on("/act", handleAction);
  server.on("/test", handleTest);
  server.on("/getChannels", handleGetChannels);
  server.on("/getTags", handleGetTags);
  server.on("/setTags", handleSetTags);
  server.on("/downloadCSV", []()
            { UpdateCSV::HandleDownloadCSV(server); });
  server.on("/uploadCSV", []()
            { UpdateCSV::HandleUploadCSV(server); });
  server.on("/loadTextFile", handleLoadTextFile);
  server.on("/saveTextFile", handleSaveTextFile);
  server.on("/edit", []()
            { HandleDataFile(server, "/text_editor.html", "text/html"); });
  server.on("/admin", []()
            { HandleDataFile(server, "/admin.html", "text/html"); });
  server.on("/otaUpdate", []()
            {
              server.send(200, "text/plain", "ESP is waiting for OTA updates...");
              isOtaOn = true;
              ArduinoOTA.begin(); });
  server.on("/chanSelMoveUp", handleChanSelMoveUp);
  server.on("/turnLater", handleTurnLater);
  server.on("/", []()
            { HandleDataFile(server, "/index.html", "text/html"); });
  server.on("/inc/style.css", []()
            { HandleDataFile(server, "/inc/style.css", "text/css"); });
  server.on("/inc/script.js", []()
            { HandleDataFile(server, "/inc/script.js", "text/javascript"); });
  // verzija 512x512 vredi zbog ikonice na Androidu; ikonica u browseru moze biti manja
  server.on("/inc/blue_remote_512.png", []()
            { HandleDataFile(server, "/inc/blue_remote_512.png", "image/png"); });
  server.on("/inc/blue_remote_48.png", []()
            { HandleDataFile(server, "/inc/blue_remote_48.png", "image/png"); });
  server.begin();
  Serial.println("HTTP server started");
  initChannels();
  irsend.begin();
  digitalWrite(pinLed, true);
}

void loop()
{
  delay(10);

  if (msSubtitles != 0 && millis() - msSubtitles > itvSubtitles)
  {
    msSubtitles = 0;
    irsend.sendNEC(Subtitle);
    delay(itvLongDelay);
    irsend.sendNEC(Subtitle);
    delay(itvLongDelay);
    irsend.sendNEC(Ok);
  }

  if (chTurnLater != -1 && millis() > msTurnLater)
  {
    turnTo(chTurnLater);
    chTurnLater = -1;
  }

  if (isOtaOn)
    ArduinoOTA.handle();
  else
    server.handleClient();

  int l = lighting.refresh();
  if (l == 1) // pali svetlo
  {
    if (lighting.getLightLevel() == 3)
      irsend.sendNEC(NecCode::HighLightOn);
    if (lighting.getLightLevel() == 2)
      irsend.sendNEC(NecCode::MediumLightOn);
  }

  if (l == -1) // gasi svetlo
    irsend.sendNEC(NecCode::LightOff);
}
