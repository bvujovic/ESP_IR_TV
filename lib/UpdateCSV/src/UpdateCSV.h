#include <ESP8266WebServer.h>

class UpdateCSV
{
private:
    // Fajl u koji se privremeno upisuje koji .csv fajl treba update-ovati sa kingtrader.info/php/
    static const String UPDATE_INI;

public:
    // Reakcija veb servera na /updateCSV?fileName=tags.csv
    static void HandleUpdateCSV(ESP8266WebServer &srv);
    // Ako UPDATE_INI postoji, upisani .csv fajl se azurira (sa kingtrader.info).
    static void DownloadNewCsvIN();
};
