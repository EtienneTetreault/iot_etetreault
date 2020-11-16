/* 
Header file for class and constant
For custom function for Etienne's clock 
Futur Comments here. */

// Etienne's library ---------------------------
#include <NTPClient.h>     // Network Time Protocol, client
#include <WiFiUdp.h>       // handles sending and receiving of UDP packages
#include <TM1637Display.h> // LED display, name of the onboard MCU

// Etienne's declaration  ---------------------------
const int red_led = D2;  //define what pin the red led is connected to
const int blue_led = D3; //define what pin the blue led is connected to

const long utcOffsetInSeconds = -18000;
WiFiUDP ntpUDP; // Define NTP Client to get time
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const int CLK = D6;              //Set the CLK pin connection to the display
const int DIO = D5;              //Set the DIO pin connection to the display
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

const int sensorPin = A0; // select the input pin for the potentiometer
const int clickPin = D8;  // Push button link to pullUp pin
int sensorValue = 30000;  // variable to store the value coming from the sensor

// unsigned long refreshTime;
unsigned long refreshTime = 30000;
unsigned long previousMillis = 0;
unsigned long previousMillisLED = 0;
int counter = 1;

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

void updateLedRed(void)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisLED >= sensorValue)
    {
        sensorValue = 30 * analogRead(sensorPin);
        Serial.println("========");
        Serial.print("sensorValue is : ");
        Serial.println(sensorValue);
        Serial.print("counterValue is : ");
        Serial.println(counter);

        if (counter % 2 == 0)
        {
            digitalWrite(red_led, HIGH);
            digitalWrite(blue_led, LOW);
            Serial.println("Red");
        }
        else
        {
            digitalWrite(red_led, LOW);
            digitalWrite(blue_led, HIGH);
            Serial.println("Blue");
        }

        counter += 1;
        previousMillisLED = currentMillis; // Remember the time
    }
}
