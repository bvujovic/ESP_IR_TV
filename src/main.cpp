#include <Arduino.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(D2);

#include "Tags.h"
#include "FS.h"

const byte pinLed = LED_BUILTIN;
const byte itvShortDelay = 50; // pauza izmedju 2 uzastopna signala
const byte itvLongDelay = 150; // dodatna pauza izmedju 2 uzastopna signala kada su susedne cifre iste
const byte cntBitsSent = 32;   // broj bita koji se salju IR putem
int cntChannels;               // broj TV kanala koji se pamte u nizu channels
int idxCurrentChannel = -1;
bool justSelectedChannels = false;

const char sepProps = '|'; // separator za razdvajanje svojstava u objektu

struct Channel
{
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
    s.concat(sepProps);
    s.concat(number);
    s.concat(sepProps);
    s.concat(spin);
    s.concat(sepProps);
    s.concat(isSelected);
    if (tags != "") // ako kanal ima tagove - dodaj ih u string
    {
      s.concat(sepProps);
      s.concat(tags);
    }
    return s;
  }
};
Channel *channels;

void initChannels()
{
  File fp = SPIFFS.open("/dat/channels.csv", "r");
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
      int idx = l.indexOf(sepProps);
      if (idx == -1)
        break;
      String strNumber = l.substring(0, idx);
      String name = l.substring(idx + 1);
      channels[i].init(name, strNumber.toInt());
      i++;
    }
    fp.close();
  }
  else
    Serial.println("channels.csv open (r) faaail.");
}

String channelsToString(bool selectedChannels = false)
{
  justSelectedChannels = selectedChannels;
  String s = "";
  for (int i = 0; i < cntChannels; i++)
    if (!selectedChannels || channels[i].isSelected)
    {
      s.concat(channels[i].toString());
      s.concat('\n');
    }
  return s;
}

void handleGetChannels()
{
  server.send(200, "text/x-csv", channelsToString());
}

void handleGetTags()
{
  server.send(200, "text/x-csv", Tags::ReadTagsFile());
}

void handleSetTags()
{
  Tags::WriteTagsFile(server.arg("plain"));
  server.send(200, "text/plain", "");
}

// test json
void handleTest()
{
  //B char s[] = "{ 'app': 'ESP_IR_TV' }";
  server.send(200, "application/json", "{ 'app': 'ESP_IR_TV' }");
}

// slanje IR kodova ka TVu
void sendIRcodes(long *codes)
{
  for (short i = 0; codes[i] != 0; i++)
  {
    if (i != 0 && codes[i] == codes[i - 1])
      delay(itvLongDelay);
    Serial.println(codes[i]);
    irsend.sendNEC(codes[i], cntBitsSent);
    delay(itvShortDelay);
  }
}

// Vraca niz kodova potrebnih za prebacivanje na dati kanal (chNumber) sa datim spinom.
// Poslednji kôd ce imati vrednost 0.
long *IRcodes(short chNumber, bool spin)
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

// Akcija daljinskog (act)
void handleAction()
{
  String cmd = server.arg("cmd");
  Serial.println(cmd);

  // gornji deo daljinskog
  if (cmd == "onOff")
    irsend.sendNEC(0x20DF10EF, cntBitsSent);
  if (cmd == "mute")
    // irsend.sendNEC(0x20DF906F, cntBitsSent); // 551522415
    irsend.sendNEC(551522415, cntBitsSent);
  if (cmd == "volUp")
  {
    irsend.sendNEC(0x20DF40BF, cntBitsSent);
    delay(itvShortDelay);
    irsend.sendNEC(0x20DF40BF, cntBitsSent);
  }
  if (cmd == "volDown")
  {
    irsend.sendNEC(0x20DFC03F, cntBitsSent);
    delay(itvShortDelay);
    irsend.sendNEC(0x20DFC03F, cntBitsSent);
  }
  if (cmd == "upCh" || cmd == "downCh")
  {
    //todo sta se sa ovim desava? da li se ovaj kôd uopste izvrsava ili se sve desava u JavaScriptu?
    Serial.print(cmd);
    Serial.print(": ");
    Serial.println(idxCurrentChannel);
    if (justSelectedChannels) // selected prikaz
    {
      byte dx = cmd == "upCh" ? +1 : -1;
    }
    else // all channels prikaz
    {
      idxCurrentChannel += cmd == "upCh" ? +1 : -1;
      idxCurrentChannel %= cntChannels;
      sendIRcodes(IRcodes(channels[idxCurrentChannel].number, channels[idxCurrentChannel].spin));
    }
  }

  // boje
  if (cmd == "colorGreen")
    irsend.sendNEC(0x20DF8E71, cntBitsSent);
  if (cmd == "colorYellow")
    irsend.sendNEC(0x20DFC639, cntBitsSent);
  // crveno 0x20DF4EB1
  // plavo  0x20DF8679

  if (cmd == "guide")
    irsend.sendNEC(0x20DFD52A, cntBitsSent);
  if (cmd == "arrRight")
    irsend.sendNEC(0x20DF609F, cntBitsSent);
  if (cmd == "back")
    irsend.sendNEC(0x20DF14EB, cntBitsSent);
  // settings 20DFC23D
  // exit     20DFDA25
  // inputs   20DFD02F
  // up       20DF02FD
  // down     20DF827D
  // left     20DFE01F
  // ok       20DF22DD

  if (cmd.startsWith("ch")) // kanali
  {
    String ch = cmd.substring(2);
    if (cmd.endsWith("_sel")) // selektovanje kanala
    {
      ch = ch.substring(0, ch.lastIndexOf("_"));
      int idx = ch.toInt();
      channels[idx].isSelected = !channels[idx].isSelected;
      Serial.println(channels[idx].isSelected);
    }
    else // aktiviranje kanala
    {
      int idx = ch.toInt();
      sendIRcodes(IRcodes(channels[idx].number, channels[idx].spin));
    }
  }

  server.send(200, "application/json", "{}");
}

// Fajl u koji se privremeno upisuje koji .csv fajl treba update-ovati sa kingtrader.info/php/
const String UPDATE_INI = "/dat/update.ini";

// /updateCSV?fileName=tags.csv
void handleUpdateCSV()
{
  String fileName = server.arg("fileName");
  Serial.println(fileName);
  File fp = SPIFFS.open(UPDATE_INI, "w");
  if (fp)
  {
    fp.print(fileName);
    fp.close();
  }
  else
    Serial.println(UPDATE_INI + " open (w) faaail.");
  delay(50);
  ESP.reset();

  server.send(200, "text/plain", "");
}

void setup()
{
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, true);

  SPIFFS.begin();
  {
    File fp = SPIFFS.open("/dat/tags.csv", "r");
    Serial.println("csv 1");
    Serial.println(fp.readString());
    fp.close();
  }
  ConnectToWiFi();

  if (SPIFFS.exists(UPDATE_INI))
  {
    Serial.println(UPDATE_INI + " postoji");
    const char *host = "kingtrader.info";
    WiFiClient client;
    if (!client.connect(host, 80))
    {
      Serial.println("connection failed");
      delay(5000);
      // return;
    }

    String csvFileName;
    // This will send a string to the server
    Serial.println("sending data to server");
    if (client.connected())
    {
      File fp = SPIFFS.open(UPDATE_INI, "r");
      String s = "";
      if (fp)
      {
        csvFileName = fp.readString();
        fp.close();
      }
      else
      {
        Serial.println(UPDATE_INI + " open (r) faaail.");
        // return;
      }

      client.print(String("GET /php/") + csvFileName + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      delay(10);
    }

    // wait for data to be available
    unsigned long timeout = millis();
    while (client.available() == 0)
      if (millis() - timeout > 5000)
      {
        Serial.println(">>> Client Timeout !");
        client.stop();
        delay(6000);
        // return;
        break;
      }

    // Read all the lines of the reply from server and print them to Serial
    Serial.println("receiving from remote server");
    
    String line;
    bool writeToFile = false;
    File fp = SPIFFS.open(String("/dat/") + csvFileName, "w");
    while (client.available())
    {
      line = client.readStringUntil('\n');
      line.trim();
      if (writeToFile)
        fp.println(line);
      if (line.length() == 0)
        writeToFile = true;
    }
    fp.close();

    // Close the connection
    Serial.println("closing connection");
    client.stop();
    SPIFFS.remove(UPDATE_INI);
  }
  else
    Serial.println(UPDATE_INI + " NE postoji");

  {
    File fp = SPIFFS.open("/dat/tags.csv", "r");
    Serial.println("csv 2");
    Serial.println(fp.readString());
    fp.close();
  }

  SetupIPAddress(20);
  server.on("/act", handleAction);
  server.on("/test", handleTest);
  server.on("/getChannels", handleGetChannels);
  server.on("/getTags", handleGetTags);
  server.on("/setTags", handleSetTags);
  server.on("/updateCSV", handleUpdateCSV);
  server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
  server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
  server.on("/inc/script.js", []() { HandleDataFile(server, "/inc/script.js", "text/javascript"); });
  server.on("/inc/blue_remote_512.png", []() { HandleDataFile(server, "/inc/blue_remote_512.png", "image/png"); });
  server.begin();
  Serial.println("HTTP server started");
  initChannels();
  irsend.begin();
}

void loop()
{
  server.handleClient();
  delay(10);
}
