#pragma once

#include <ESP8266WebServer.h>

class UpdateCSV
{
private:
    // Fajl u koji se privremeno upisuje koji .csv fajl treba download-ovati sa kingtrader.info/php/
    static const String DOWNLOAD_INI;
    // Fajl u koji se privremeno upisuje koji .csv fajl treba upload-ovati na kingtrader.info/php/
    static const String UPLOAD_INI;
    // kingtrader.info
    static const String WEB_HOST;
    // Veb stranica koja obavestava korisnika da mora malo da saceka.
    static const String WAIT_HTML;
    
    static String UrlEncode(String str);

public:
    // Reakcija veb servera na /downloadCSV?fileName=tags.csv
    static void HandleDownloadCSV(ESP8266WebServer &srv);
    // Ako DOWNLOAD_INI postoji, upisani .csv fajl se azurira (sa kingtrader.info).
    static void DownloadNewCsvIN();
    // Upload nekog .csv fajla na kingtrader.info
    static void HandleUploadCSV(ESP8266WebServer &srv);
    // Ako UPLOAD_INI postoji, upisani .csv fajl se salje na kingtrader.info.
    static void UploadNewCsvIN();
};
