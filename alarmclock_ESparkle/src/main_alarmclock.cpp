//  Inspired of See https://github.com/CosmicMac/ESParkle

/*  
Deleted some functionnality, see later if require :
- Accelerometer control : Na...
- LED control with hardcoded pattern via FastLed library : 
*/

//#define USE_I2S                 // uncomment to use extenral I2S DAC

#include <Arduino.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
// #include <MPU6050.h>
#include <FastLED.h>
#include <Ticker.h>
#include <AudioFileSourceHTTPStream.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorRTTTL.h>
#include <AudioGeneratorMP3.h>
#ifdef USE_I2S
#include "AudioOutputI2S.h"
#else
#include "AudioOutputI2SNoDAC.h"
#endif
#include "esparkle.h"
#include "config.h"
// Etienne's library ---------------------------
#include <NTPClient.h>     // Network Time Protocol, client
#include <WiFiUdp.h>       // handles sending and receiving of UDP packages
#include <TM1637Display.h> // LED display, name of the onboard MCU
#include "LedControl.h"

// Etienne's declaration  ---------------------------
LedControl led_controller({D2, D3, D4}, "mqttLedState"); // Instance of LedControl class
char led_payload[32] = "";

const long utcOffsetInSeconds = -18000;
WiFiUDP ntpUDP;                                                   // Define NTP Client to get time
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); // Instance of timeclock client

const int CLK = D6;              //Set the CLK pin connection to the display
const int DIO = D5;              //Set the DIO pin connection to the display
TM1637Display display(CLK, DIO); // Instance of clock display class

unsigned long clock_refresh_period = 30000;
unsigned long lastClockMillis = 0;
unsigned long lastDebugMillis = 0;

// Misc global variables
bool otaInProgress = false;
bool ledActionInProgress = false;
bool newAudioSource = false;

CRGB leds[NUM_LEDS];
int curColor;
uint32_t curDelay;

char audioSource[256] = "";
uint8_t msgPriority = 0;
float onceGain = 0;

ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker ledActionTimer;

AudioFileSourceHTTPStream *stream = nullptr;
AudioFileSourceSPIFFS *file = nullptr;
AudioFileSourceBuffer *buff = nullptr;
AudioFileSourcePROGMEM *string = nullptr;
AudioGeneratorMP3 *mp3 = nullptr;
AudioGeneratorRTTTL *rtttl = nullptr;
#ifdef USE_I2S
AudioOutputI2S *out = NULL;
#else
AudioOutputI2SNoDAC *out = NULL;
#endif
//############################################################################
// SETUP
//############################################################################

void setup()
{
  Serial.begin(115200);

  // INIT SPIFFS
  SPIFFS.begin();

  // INIT WIFI
  WiFi.hostname(ESP_NAME);
  WiFi.mode(WIFI_STA);
  for (auto i : AP_LIST)
  {
    wifiMulti.addAP(i.ssid, i.passphrase);
  }
  wifiConnect();

  // INIT MQTT
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttConnect(true);

  // INIT OTA
  ArduinoOTA.setHostname(ESP_NAME);
  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    Serial.println(F("Start updating..."));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.println(progress / (total / 100));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nend"));
    otaInProgress = false;
  });
  ArduinoOTA.onError([](ota_error_t error) {
    String msg;
    if (error == OTA_AUTH_ERROR)
      msg = F("auth failed");
    else if (error == OTA_BEGIN_ERROR)
      msg = F("begin failed");
    else if (error == OTA_CONNECT_ERROR)
      msg = F("connect failed");
    else if (error == OTA_RECEIVE_ERROR)
      msg = F("receive failed");
    else if (error == OTA_END_ERROR)
      msg = F("end failed");
    Serial.printf_P(PSTR("Error: %s"), msg.c_str());
  });
  ArduinoOTA.begin();

  // INIT LED
  LEDS.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  ledDefault();

  // Etienne's setup ---------------------------
  timeClient.begin();
  display.clear();
  display.setBrightness(1); // Set the brightness:
  display.showNumberDecEx(1234, 0b11100000, false, 4, 0);
  led_controller.setUpInitialize();
}

//############################################################################
// LOOP
//############################################################################

void loop()
{
  unsigned long curMillis = millis();

  // Etienne LedControl.h object main loop
  led_controller.loopTimer();

  // Etienne Display Clock main loop
  if (curMillis - lastClockMillis >= clock_refresh_period)
  {
    timeClient.update();
    int timeNow = timeClient.getHours() * 100 + timeClient.getMinutes();
    display.showNumberDecEx(timeNow, 0b01000000, false, 4, 0);
    lastClockMillis = curMillis; // Remember the time
  }

  if (curMillis - lastDebugMillis >= 2000)
  {
    Serial.printf_P(PSTR("\nFree heap: %d\n"), ESP.getFreeHeap());
    lastDebugMillis = curMillis; // Remember the time
  }

  // HANDLE Wifi
  static unsigned long lastWifiMillis = 0;
  bool wifiIsConnected = WiFi.isConnected();
  if (!wifiIsConnected)
  {
    stopPlaying();

    // ledBlink(50, 0xFF0000);
    if (curMillis - lastWifiMillis > 60000)
    {
      if (wifiConnect())
      {
        // ledDefault();
      }
      lastWifiMillis = curMillis;
    }
  }

  // HANDLE OTA
  if (wifiIsConnected)
  {
    ArduinoOTA.handle();
    if (otaInProgress)
    {
      return;
    }
  }

  // HANDLE MQTT
  static unsigned long lastMqttConnMillis = 0;
  if (wifiIsConnected)
  {
    if (!mqttClient.connected())
    {
      if (curMillis - lastMqttConnMillis > 5000)
      {
        Serial.println(F("Disconnected from MQTT"));
        mqttConnect();
        lastMqttConnMillis = curMillis;
      }
    }
    else
    {
      mqttClient.loop();
    }
  }

  // HANDLE MP3
  if (newAudioSource)
  {
    playAudio();
  }
  else if (mp3 && mp3->isRunning())
  {
    if (!mp3->loop())
    {
      //mp3->stop();
      stopPlaying();
      //Serial.println(F("MP3 done"));
    }
  }
  else if (rtttl && rtttl->isRunning())
  {
    if (!rtttl->loop())
    {
      //rtttl->stop();
      stopPlaying();
      //Serial.println(F("RTTTL done"));
    }
  }

  // HANDLE LED
  FastLED.show();
}

//############################################################################
// WIFI
//############################################################################

bool wifiConnect()
{
  Serial.print(F("Connecting to WiFi"));
  uint8_t count = 10;
  while (count-- && (wifiMulti.run() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(1000);
  }

  if (WiFi.isConnected())
  {
    Serial.println(F("\nConnected to WiFi"));
    return true;
  }
  else
  {
    Serial.println(F("\nUnable to connect to WiFi"));
    return false;
  }
}

//############################################################################
// MQTT
//############################################################################

bool mqttConnect(bool about)
{

  String cltName = String(ESP_NAME) + '_' + String(ESP.getChipId(), HEX);
  if (mqttClient.connect(cltName.c_str(), MQTT_USER, MQTT_PASSWORD))
  {
    Serial.println(F("Connected to MQTT"));
    if (about)
    {
      mqttCmdAbout();
    }
    else
    {
      mqttClient.publish(MQTT_OUT_TOPIC, PSTR("Reconnected to MQTT"));
    }
    mqttClient.subscribe(MQTT_IN_TOPIC);
    mqttClient.subscribe(led_controller.mqtt_topic.c_str());
  }
  else
  {
    Serial.println(F("Unable to connect to MQTT"));
  }
  return mqttClient.connected();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  StaticJsonBuffer<512> jsonInBuffer;
  JsonObject &json = jsonInBuffer.parseObject(payload);

  if (!json.success())
  {
    mqttClient.publish(MQTT_OUT_TOPIC, PSTR("{event:\"jsonInBuffer.parseObject(payload) failed\"}"));
    return;
  }

  json.printTo(Serial);
  Serial.println();

  //Etienne led MQTT to controler
  if (json.containsKey("etienne_led"))
  {
    strlcpy(led_payload, json["etienne_led"], sizeof(led_payload));
    led_controller.getMqttUpdate(led_payload);
  }

  // Simple commands
  if (json.containsKey("cmd"))
  {
    if (strcmp("break", json["cmd"]) == 0)
    { // Break current action: {cmd:"break"}
      stopPlaying();
      /*
            if (mp3 && mp3->isRunning()) {
                mp3->stop();
            } else if (rtttl && rtttl->isRunning()) {
                rtttl->stop();
            }
             */
      if (ledActionInProgress)
      {
        msgPriority = 0;
        ledDefault();
      }
    }
    else if (strcmp("restart", json["cmd"]) == 0)
    { // Restart ESP: {cmd:"restart"}
      ESP.restart();
      delay(500);
    }
    else if (strcmp("about", json["cmd"]) == 0)
    { // About: {cmd:"about"}
      mqttCmdAbout();
    }
    else if (strcmp("list", json["cmd"]) == 0)
    { // List SPIFFS files: {cmd:"list"}
      mqttCmdList();
    }
    return;
  }

  // Set max brightness: {bright:255}
  if (json.containsKey("bright"))
  {
    uint8_t b = json["bright"].as<uint8_t>();
    if (b >= 0 && b <= 255)
    {
      FastLED.setBrightness(b);
    }
  }

  // Set default gain: {"gain":1.2}
  if (json.containsKey("gain"))
  {
    float g = json["gain"].as<float>();
    if (g > 0.01 && g < 3.0)
    {
      defaultGain = g;
    }
  }

  // Set once gain: {oncegain:1.2}
  if (json.containsKey("oncegain"))
  {
    float g = json["oncegain"].as<float>();
    if (g > 0.01 && g < 3.0)
    {
      onceGain = g;
    }
  }

  // Set new MP3 source
  // - from stream: {"mp3":"http://www.universal-soundbank.com/sounds/7340.mp3"}
  // - from SPIFFS: {"mp3":"/mp3/song.mp3"}
  if (json.containsKey("mp3"))
  {
    strlcpy(audioSource, json["mp3"], sizeof(audioSource));
    newAudioSource = true;
  }

  // Set new MP3 source from TTS proxy {"tts":"May the force be with you"}
  if (json.containsKey("tts"))
  {
    tts(json["tts"], json.containsKey("voice") ? json["voice"] : String());
  }

  // Set new Rtttl source {"rtttl":"Xfiles:d=4,o=5,b=160:e,b,a,b,d6,2b."}
  if (json.containsKey("rtttl"))
  {
    strlcpy(audioSource, json["rtttl"], sizeof(audioSource));
    newAudioSource = true;
  }

  // Set new message priority : {"led":"Blink",color:"0xff0000",delay:50,priority:9}
  // LED pattern is considered only if msg priority is >= to previous msg priority
  // This is to avoid masking an important LED alert with a minor one
  if (json.containsKey("priority"))
  {
    uint8_t thisPriority = json["priority"].as<uint8_t>();
    if (thisPriority < msgPriority)
    {
      return;
    }
    msgPriority = thisPriority;
  }

  // Set led pattern: {"led":"Blink",color:"0xff0000",delay:50}
  if (json.containsKey("led"))
  {
    uint32_t d = 100;
    if (json.containsKey("delay"))
    {
      d = json["delay"].as<uint32_t>();
    }

    int c = 0xFFFFFF;
    if (json.containsKey("color"))
    {
      c = (int)strtol(json["color"], nullptr, 0);
    }

    if (strcmp("Rainbow", json["led"]) == 0)
    {
      ledRainbow(d);
    }
    else if (strcmp("Blink", json["led"]) == 0)
    {
      ledBlink(d, c);
    }
    else if (strcmp("Sine", json["led"]) == 0)
    {
      ledSine(d, c);
    }
    else if (strcmp("Pulse", json["led"]) == 0)
    {
      ledPulse(d, c);
    }
    else if (strcmp("Disco", json["led"]) == 0)
    {
      ledDisco(d);
    }
    else if (strcmp("Solid", json["led"]) == 0)
    {
      ledSolid(c);
    }
    else if (strcmp("Off", json["led"]) == 0)
    {
      ledOff();
    }
    else
    {
      ledDefault();
    }
  }
}

/**
 * Publish useful ESP information to MQTT out topic
 */
void mqttCmdAbout()
{
  String freeSpace;
  prettyBytes(ESP.getFreeSketchSpace(), freeSpace);

  String sketchSize;
  prettyBytes(ESP.getSketchSize(), sketchSize);

  String chipSize;
  prettyBytes(ESP.getFlashChipRealSize(), chipSize);

  String freeHeap;
  prettyBytes(ESP.getFreeHeap(), freeHeap);

  Serial.println(F("Preparing about..."));

  DynamicJsonBuffer jsonBuffer;

  JsonObject &jsonRoot = jsonBuffer.createObject();
  jsonRoot[F("version")] = ESPARKLE_VERSION;
  jsonRoot[F("sdkVersion")] = ESP.getSdkVersion();
  jsonRoot[F("coreVersion")] = ESP.getCoreVersion();
  jsonRoot[F("resetReason")] = ESP.getResetReason();
  jsonRoot[F("ssid")] = WiFi.SSID();
  jsonRoot[F("ip")] = WiFi.localIP().toString();
  jsonRoot[F("staMac")] = WiFi.macAddress();
  jsonRoot[F("apMac")] = WiFi.softAPmacAddress();
  jsonRoot[F("chipId")] = String(ESP.getChipId(), HEX);
  jsonRoot[F("chipSize")] = chipSize;
  jsonRoot[F("sketchSize")] = sketchSize;
  jsonRoot[F("freeSpace")] = freeSpace;
  jsonRoot[F("freeHeap")] = freeHeap;
  jsonRoot[F("defaultGain")] = defaultGain;

  String mqttMsg;
  jsonRoot.prettyPrintTo(mqttMsg);
  Serial.println(mqttMsg.c_str());

  mqttClient.publish(MQTT_OUT_TOPIC, mqttMsg.c_str());
}

void mqttCmdList()
{

  DynamicJsonBuffer jsonBuffer;
  JsonArray &array = jsonBuffer.createArray();

  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    array.add(dir.fileName());
  }

  String mqttMsg;
  array.prettyPrintTo(mqttMsg);

  mqttClient.publish(MQTT_OUT_TOPIC, mqttMsg.c_str());
}

//############################################################################
// AUDIO
//############################################################################

void playAudio()
{
  newAudioSource = false;

  if (audioSource[0] == 0)
  {
    return;
  }

  stopPlaying();

  Serial.printf_P(PSTR("\nFree heap: %d\n"), ESP.getFreeHeap());

  if (!out)
  {
#ifdef USE_I2S
    out = new AudioOutputI2S();
    Serial.println("Using I2S output");
#else
    out = new AudioOutputI2SNoDAC();
    Serial.println("Using No DAC - using Serial port Rx pin");
#endif
    out->SetOutputModeMono(true);
  }
  out->SetGain(onceGain ?: defaultGain);
  onceGain = 0;

  if (strncmp("http", audioSource, 4) == 0)
  {
    // Get MP3 from stream
    Serial.printf_P(PSTR("**MP3 stream: %s\n"), audioSource);
    stream = new AudioFileSourceHTTPStream(audioSource);
    buff = new AudioFileSourceBuffer(stream, 1024 * 2);
    //buff = new AudioFileSourceBuffer(stream, preallocateBuffer, preallocateBufferSize);
    mp3 = new AudioGeneratorMP3();
    mp3->begin(buff, out);
    if (!mp3->isRunning())
    {
      //Serial.println(F("Unable to play MP3"));
      stopPlaying();
    }
  }
  else if (strncmp("/mp3/", audioSource, 5) == 0)
  {
    // Get MP3 from SPIFFS
    Serial.printf_P(PSTR("**MP3 file: %s\n"), audioSource);
    file = new AudioFileSourceSPIFFS(audioSource);
    mp3 = new AudioGeneratorMP3();
    mp3->begin(file, out);
    if (!mp3->isRunning())
    {
      //Serial.println(F("Unable to play MP3"));
      stopPlaying();
    }
  }
  else
  {
    // Get RTTTL
    Serial.printf_P(PSTR("**RTTL file: %s\n"), audioSource);
    string = new AudioFileSourcePROGMEM(audioSource, strlen(audioSource));
    rtttl = new AudioGeneratorRTTTL();
    rtttl->begin(string, out);
    if (!rtttl->isRunning())
    {
      //Serial.println(F("Unable to play RTTTL"));
      stopPlaying();
    }
  }
}

bool stopPlaying()
{
  bool stopped = false;
  if (rtttl)
  {
    rtttl->stop();
    delete rtttl;
    rtttl = nullptr;
    stopped = true;
  }
  if (mp3)
  {
    mp3->stop();
    delete mp3;
    mp3 = nullptr;
    stopped = true;
  }
  if (buff)
  {
    buff->close();
    delete buff;
    buff = nullptr;
  }
  if (file)
  {
    file->close();
    delete file;
    file = nullptr;
  }
  if (stream)
  {
    stream->close();
    delete stream;
    stream = nullptr;
  }
  if (string)
  {
    string->close();
    delete string;
    string = nullptr;
  }

  return stopped;
}

void tts(String text, String voice)
{
  if (text.length())
  {
    String query = String("text=") + text;
    if (voice.length())
    {
      query += "&voice=" + voice;
    }
    Serial.println(query.c_str());
    HTTPClient http;
    http.begin(espClient, TTS_PROXY_URL);
    http.setAuthorization(TTS_PROXY_USER, TTS_PROXY_PASSWORD);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(query);
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println(payload.c_str());
      if (payload.endsWith(".mp3"))
      {
        strlcpy(audioSource, payload.c_str(), sizeof(audioSource));
        newAudioSource = true;
      }
    }
    else
    {
      String payload = http.getString();
      Serial.println(payload.c_str());
    }
  }
}

//############################################################################
// LED
//############################################################################

void ledDefault(uint32_t delay)
{

  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = false;
    static uint8_t hue = 0;
    fill_solid(leds, NUM_LEDS, CHSV(hue++, 255, 255));
  });
}

void ledRainbow(uint32_t delay)
{

  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = true;
    static uint8_t hue = 0;
    fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
    hue = ++hue % 255;
  });
}

void ledBlink(uint32_t delay, int color)
{

  curColor = color;

  Serial.println(curColor, HEX);

  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = true;
    static bool t = true;
    fill_solid(leds, NUM_LEDS, t ? curColor : CRGB::Black);
    t = !t;
  });
}

void ledSine(uint32_t delay, int color)
{

  curColor = color;

  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = true;

    static uint8_t i = 0;
    static CHSV hsvColor;
    if (curColor)
    {
      hsvColor = rgb2hsv_approximate(curColor);
      curColor = 0;
    }
    hsvColor.value = sin8(i);
    fill_solid(leds, NUM_LEDS, hsvColor);
    i = (i + 1) % 255;
  });
}

void ledPulse(uint32_t delay, int color)
{

  curColor = color;
  curDelay = delay;

  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = true;

    static uint16_t blackCountdown = 0;

    if (blackCountdown)
    {
      blackCountdown--;
      return;
    }

    static uint8_t i = 0;
    CRGB rgbColor = curColor;

    rgbColor.fadeToBlackBy(i);
    fill_solid(leds, NUM_LEDS, rgbColor);
    i = (i + 1) % 255;

    if (i == 0)
    {
      blackCountdown = 750 / curDelay; // Stay black during 750ms
    }
  });
}

void ledDisco(uint32_t delay)
{
  ledActionTimer.detach();
  ledActionTimer.attach_ms(delay, []() {
    ledActionInProgress = true;
    fill_solid(leds, NUM_LEDS, CHSV(random8(), 255, 255));
  });
}

void ledSolid(int color)
{
  ledActionTimer.detach();
  fill_solid(leds, NUM_LEDS, color);
}

void ledOff()
{
  ledSolid(0x000000);
}

//############################################################################
// HELPERS
//############################################################################

void prettyBytes(uint32_t bytes, String &output)
{

  const char *suffixes[7] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
  uint8_t s = 0;
  double count = bytes;

  while (count >= 1024 && s < 7)
  {
    s++;
    count /= 1024;
  }
  if (count - floor(count) == 0.0)
  {
    output = String((int)count) + suffixes[s];
  }
  else
  {
    output = String(round(count * 10.0) / 10.0, 1) + suffixes[s];
  };
}
