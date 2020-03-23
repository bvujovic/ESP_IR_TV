#include "UpdateCSV.h"
#include <Arduino.h>
#include <SpiffsUtils.h>

const String UpdateCSV::DOWNLOAD_INI = "/dat/download.ini";
const String UpdateCSV::UPLOAD_INI = "/dat/upload.ini";
const String UpdateCSV::WEB_HOST = "kingtrader.info";
const String UpdateCSV::WAIT_HTML = "/wait.html";

void UpdateCSV::HandleDownloadCSV(ESP8266WebServer &server)
{
    String fileName = server.arg("fileName");
    //T Serial.println(fileName);

    SpiffsUtils::WriteFile(DOWNLOAD_INI, fileName);
    delay(10);
    server.send(200, "text/html", SpiffsUtils::ReadFile(WAIT_HTML));
    server.stop();
    delay(200);
    ESP.reset();
}

void UpdateCSV::DownloadNewCsvIN()
{
    if (!SPIFFS.exists(DOWNLOAD_INI))
        return;

    Serial.println(DOWNLOAD_INI + " postoji");
    WiFiClient client;
    if (!client.connect(WEB_HOST, 80))
    {
        Serial.println("DownloadNewCsvIN - connection failed");
        return;
    }

    String csvFileName;
    //T Serial.println("sending data to server");
    if (client.connected())
    {
        csvFileName = SpiffsUtils::ReadFile(DOWNLOAD_INI);
        if (csvFileName.length() == 0)
            return;

        client.print(String("GET /php/") + csvFileName + " HTTP/1.1\r\n" +
                     "Host: " + WEB_HOST + "\r\n" +
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
    SPIFFS.remove(DOWNLOAD_INI);
    //T Serial.println("DownloadNewCsvIN - End.");
}

void UpdateCSV::HandleUploadCSV(ESP8266WebServer &server)
{
    String fileName = server.arg("fileName");
    Serial.println(fileName);

    SpiffsUtils::WriteFile(UPLOAD_INI, fileName);
    delay(10);
    server.send(200, "text/html", SpiffsUtils::ReadFile(WAIT_HTML));
    server.stop();
    delay(200);
    ESP.reset();
}

void UpdateCSV::UploadNewCsvIN()
{
    if (!SPIFFS.exists(UPLOAD_INI))
        return;

    //T Serial.println(UPLOAD_INI + " postoji");
    String csvFileName = SpiffsUtils::ReadFile(UPLOAD_INI);
    if (csvFileName.length() == 0)
        return;
    //T Serial.println(csvFileName);

    WiFiClient client;
    if (!client.connect(WEB_HOST, 80))
    {
        //T Serial.println("UploadNewCsvIN - connection failed");
        return;
    }

    if (client.connected())
    {
        String csvContent = UrlEncode(SpiffsUtils::ReadFile("/dat/" + csvFileName));
        String content = "fileName=" + csvFileName + "&txt=" + csvContent;
        Serial.println(content);
        client.print(String("POST /php/esp_ir_tv_csv_upload.php HTTP/1.1\r\n") +
                     "Host: " + WEB_HOST + "\r\n" +
                     "Content-Type: application/x-www-form-urlencoded\r\n" +
                     "Content-Length: " + content.length() + "\r\n\r\n" +
                     content);
        delay(10);
    }

    client.stop();
    SPIFFS.remove(UPLOAD_INI);
    //T Serial.println("UploadNewCsvIN kraj");
}

// Preuzeto sa https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
String UpdateCSV::UrlEncode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
        yield();
    }
    return encodedString;
}