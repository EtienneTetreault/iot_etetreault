/* 
Header file for class and constant
For custom function for Etienne's clock 
Futur Comments here. */

// Etienne's library ---------------------------
#include <NTPClient.h>     // Network Time Protocol, client
#include <WiFiUdp.h>       // handles sending and receiving of UDP packages
#include <TM1637Display.h> // LED display, name of the onboard MCU

// Etienne's declaration  ---------------------------

const long utcOffsetInSeconds = -18000;
WiFiUDP ntpUDP; // Define NTP Client to get time
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const int CLK = D6;              //Set the CLK pin connection to the display
const int DIO = D5;              //Set the DIO pin connection to the display
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

unsigned long refreshTime = 30000;
unsigned long previousMillis = 0;

// Etienne's Functions -------------
void updateEtiClock(void)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= refreshTime)
    {
        timeClient.update();
        int timeNow = timeClient.getHours() * 100 + timeClient.getMinutes();
        display.showNumberDecEx(timeNow, 0b01000000, false, 4, 0);
        previousMillis = currentMillis; // Remember the time
    }
}
