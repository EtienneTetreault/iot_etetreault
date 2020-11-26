#include <Arduino.h>

#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char *mqtt_server = "IP_ADRESS";
// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient mqttclient(espClient);

WiFiManager wifiManager;

// Lamp - LED - GPIO 4 = D2 on ESP-12E NodeMCU board
const int lamp = D1;
unsigned long clock_prev_millis = 0;

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if (topic == "room/lamp")
  {
    Serial.print("Changing Room lamp to ");
    if (messageTemp == "on")
    {
      digitalWrite(lamp, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "off")
    {
      digitalWrite(lamp, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266
void mqttReconnect()
{
  // Loop until we're reconnected
  while (!mqttclient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (mqttclient.connect("ESP8266Client")) {
     You can do it like this:
       if (mqttclient.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (mqttclient.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (mqttclient.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      mqttclient.subscribe("remoteScale/fromNode");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  /* ############################ WifiManager ############################################# */

  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  Serial.println("connected...yeey :)"); //if you get here you have connected to the WiFi

  pinMode(lamp, OUTPUT);
  digitalWrite(lamp, LOW);
  Serial.begin(115200);
  mqttclient.setServer(mqtt_server, 1883);
  mqttclient.setCallback(callback);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!mqttclient.connected())
  {
    mqttReconnect();
  }
  if (!mqttclient.loop())
    mqttclient.connect("ESP8266Client");

  // Publishes to MQTT after period
  unsigned long clock_current_millis = millis();
  if (clock_current_millis - clock_prev_millis >= 5000)
  {
    mqttclient.publish("remoteScale/fromESP", "FromESPmini Yeah!!");
    Serial.print("Message sent: ");
    clock_prev_millis = clock_current_millis; // Remember the time
  }
}