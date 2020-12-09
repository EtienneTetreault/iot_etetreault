#ifndef ESPARKLE_H
#define ESPARKLE_H

// Mostly forward declaration of functions use in main.cpp
bool wifiConnect();

void playAudio();
bool stopPlaying();
void tts(String text, String voice);

bool mqttConnect(bool about = false);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttCmdAbout();
void mqttCmdList();

void ledDefault(uint32_t delay = 500);
void ledRainbow(uint32_t delay);
void ledBlink(uint32_t delay, int color);
void ledSine(uint32_t delay, int color);
void ledPulse(uint32_t delay, int color);
void ledDisco(uint32_t delay);
void ledSolid(int color);
void ledOff();

void prettyBytes(uint32_t bytes, String &output);
#endif //ESPARKLE_H
