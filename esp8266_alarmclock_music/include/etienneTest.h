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
    unsigned long period_millis = 5000;
    unsigned long previous_millis = 0; // TODO check for better time counter, what happen when overflow?
    std::vector<int> led_state_arr = {0, 0, 0};

    // Constant attributes
    const std::vector<int> led_pin_arr = {D4, D2, D3};
    const std::string mqtt_topic = "emptyStr"; // TODO : Integrate mqtt topic from class to the broker

    // Constructor
    LedControl();
    LedControl(std::vector<int> led_pin_arr, std::string t_mqtt_topic);

    // Methods
    void getMqttUpdate(char t_payload_led[]);
    void setUpInitialize(void);
    void updateLed(void);
    void setStateControl(int t_int_of_req_state);
    void loopTimer(void);
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
    int temp_state;
    sscanf(temp_token, "%d", &temp_state);

    // Second token/part of string is the period require
    int temp_period;
    temp_token = strtok(NULL, ",");
    sscanf(temp_token, "%d", &temp_period);
    period_millis = temp_period;

    // Send required int/state from the MQTT to the setting method of the class
    setStateControl(temp_state);
}

void LedControl::setStateControl(int t_int_of_req_state)
{
    if (t_int_of_req_state <= NB_STATES)
    {
        state_control = static_cast<LedControlState>(t_int_of_req_state);
    }
    else
    {
        state_control = RESETTING; // TODO : Put at RESET state_control?? And throw error??
        Serial.println("Wrong State_control from MQTT");
    }

    // state_control = t_state_to_set;
    // Set required attributes according to state machine
    switch (state_control)
    {
    case IDLE:
        /* code */
        break;

    // TODO : No different state for Led On, but send MQTT number to recreate onboard the led_state_arr mask (nLed + nLed desiredOn)
    case LED_OVERLAP:
        led_state_arr = {1, 1, 0};
        break;

    case LED_NO_OVERLAP:
        led_state_arr = {1, 0, 0};
        break;

    case RESETTING:
        led_state_arr = {0, 0, 0};
        updateLed();
        state_control = IDLE;
        period_millis = 5000;
        previous_millis = 0;
        // TODO : Send confirmation via MQTT to Node that all is stop??
        break;

    default:
        break;
    }
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

void LedControl::loopTimer(void)
{
    if (state_control > IDLE)
    {
        unsigned long current_millis = millis();
        if (current_millis - previous_millis >= period_millis)
        {
            updateLed();
            previous_millis = current_millis;
        }
    }
}

LedControl led_controller; // Instance of LedControl class