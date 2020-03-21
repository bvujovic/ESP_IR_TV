#include "UpdateCSV.h"
#include <Arduino.h>
#include <SpiffsUtils.h>

const String UpdateCSV::UPDATE_INI = "/dat/update.ini";

void UpdateCSV::HandleUpdateCSV(ESP8266WebServer &server)
{
    String fileName = server.arg("fileName");
    Serial.println(fileName);

    SpiffsUtils::WriteFile(UPDATE_INI, fileName);
    delay(10);

    Serial.println("sending...");
    const char html[] = R"=====(
<HTML>
<HEAD>
    <TITLE>TV ctrl</TITLE>
    <script> function f() { window.location.href = '/'; } </script>
</HEAD>
<BODY onload="setTimeout(f, 5000);">
    <B>App will be refreshed in less than 5secs.</B>
</BODY>
</HTML>
)=====";
    server.send(200, "text/html", html);
    Serial.println("sent");
    server.stop();
    Serial.println("stopped");
    delay(200);
    ESP.reset();
}

void UpdateCSV::DownloadNewCsvIN()
{
    if (!SPIFFS.exists(UPDATE_INI))
        return;

    Serial.println(UPDATE_INI + " postoji");
    const char *host = "kingtrader.info";
    WiFiClient client;
    if (!client.connect(host, 80))
    {
        Serial.println("DownloadNewCsvIN - connection failed");
        return;
    }

    String csvFileName;
    //T Serial.println("sending data to server");
    if (client.connected())
    {
        csvFileName = SpiffsUtils::ReadFile(UPDATE_INI);
        if (csvFileName.length() == 0)
            return;

        client.print(String("GET /php/") + csvFileName + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");
        delay(10);
    }

    unsigned long timeout = millis();
    while (client.available() == 0)
        if (millis() - timeout > 5000)
        {
            Serial.println("DownloadNewCsvIN - Client Timeout !");
            client.stop();
            //B delay(6000);
            return;
        }

    //T Serial.println("receiving data...");
    //T Serial.println(csvFileName);
    String line;
    bool writeToFile = false;
    File fp = SPIFFS.open(String("/dat/") + csvFileName, "w");
    while (client.available())
    {
        line = client.readStringUntil('\n');
        line.trim();
        if (writeToFile)
        {
            //T Serial.println(line);
            fp.println(line);
        }
        if (line.length() == 0)
            writeToFile = true;
    }
    fp.close();

    //T Serial.println("closing connection");
    client.stop();
    SPIFFS.remove(UPDATE_INI);
    //T Serial.println("DownloadNewCsvIN - End.");
}