// #include <Arduino.h>
// #include <WiFiServerBasics.h>
// #include "Lighting.h"
// Lighting lighting;

// void setup()
// {
//     Serial.begin(115200);
//     Serial.println();
//     WiFi.mode(WIFI_STA);
//     ConnectToWiFi();
//     Serial.println("Connected");
//     lighting.init(true, 4, 23, 4, 30, 2);
// }

// void loop()
// {
//     int l = lighting.refresh();
//     if (l == 1)
//         Serial.println("pali");
//     if (l == -1)
//         Serial.println("gasi");
//     delay(50);
// }