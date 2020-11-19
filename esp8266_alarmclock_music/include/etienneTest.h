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
    int period_milli = 5000;
    unsigned long previous_millis = 0; // TODO check for better time counter, what happen when overflow?
    std::string mqtt_topic = "emptyStr";
    std::vector<int> led_pin_arr = {D4, D2, D3};
    std::vector<int> led_state_arr = {0, 0, 0};

    // Constructor
    LedControl();
    LedControl(std::vector<int> led_pin_arr, std::string t_mqtt_topic);

    // Methods
    void getMqttUpdate(char t_payload_led[]);
    void setUpInitialize(void);
    void updateLed(void);
};

LedControl::LedControl() = default;

LedControl::LedControl(std::vector<int> t_led_pin_arr, std::string t_mqtt_topic)
    : led_pin_arr(t_led_pin_arr), mqtt_topic(t_mqtt_topic)
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

    for (auto &i : led_pin_arr)
    {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
    }
}

void LedControl::updateLed(void)
{
    // Shift the led state arr / mask to one position
    std::rotate(led_state_arr.begin(), led_state_arr.begin() + 1, led_state_arr.end());

    // Apply de led state arr to each pin
    for (std::vector<int>::size_type i = 0; i != led_pin_arr.size(); i++)
    {
        digitalWrite(led_pin_arr[i], led_state_arr[i]);
    }
}

LedControl led_controller; // Instance of LedControl class