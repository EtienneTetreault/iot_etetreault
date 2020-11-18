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
        if (digitalRead(red_led) == LOW)
        {
            digitalWrite(red_led, HIGH);
            digitalWrite(blue_led, LOW);
        }
        else
        {
            digitalWrite(red_led, LOW);
            digitalWrite(blue_led, HIGH);
        }

        previousMillisLED = currentMillis; // Remember the time
    }
}

enum LedControlState
{
    IDLE,
    LED_OVERLAP,
    LED_NO_OVERLAP,
    RESETTING,
    NB_STATES
};

class LedControl
{
public:
    LedControlState state_control = IDLE;
    int period_milli = 10000;
    const int pin_blue = D1;
    const int pin_green = D2;
    const int pin_white = D3;
    int state_blue = LOW;
    int state_green = LOW;
    int state_white = LOW;
    unsigned long previous_millis = 0; // TODO check for better time counter, what happen when overflow?
    std::string mqtt_topic = "emptyStr";

    // Constructor
    LedControl();
    LedControl(int t_blue_pin, int t_green_pin, int t_white_pin, std::string t_mqtt_topic);

    // Methods
    void getMqttUpdate(char t_payload_led[]);
    void setUpInitialize(void);
};

LedControl::LedControl() = default;

LedControl::LedControl(int t_blue_pin, int t_green_pin, int t_white_pin, std::string t_mqtt_topic)
    : pin_blue(t_blue_pin),
      pin_green(t_green_pin), pin_white(t_white_pin), mqtt_topic(t_mqtt_topic)
{
}

void LedControl::getMqttUpdate(char t_payload_led[])
{
    // First token/part of string is the state_control
    char *temp_token = strtok(t_payload_led, ",");
    int temp_int;
    sscanf(temp_token, "%d", &temp_int);
    if (temp_int <= NB_STATES)
    {
        state_control = static_cast<LedControlState>(temp_int);
    }
    else
    {
        state_control = IDLE; // TODO : Put at RESET state_control?? And throw error??
        Serial.println("Wrong State_control from MQTT");
    }

    // Second token/part of string is the period require
    temp_token = strtok(NULL, ",");
    sscanf(temp_token, "%d", &temp_int);
    period_milli = temp_int;
}

void LedControl::setUpInitialize(void)
{
    pinMode(pin_blue, OUTPUT);
    pinMode(pin_green, OUTPUT);
    pinMode(pin_white, OUTPUT);
    digitalWrite(pin_blue, LOW);
    digitalWrite(pin_green, LOW);
    digitalWrite(pin_white, LOW);
}

LedControl led_controller; // Instance of LedControl class