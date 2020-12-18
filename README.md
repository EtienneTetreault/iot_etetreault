# iot_etetreault
Repository for all personnal IoT related script, specifically embedded microcontroler source files.

## Current release [0.2.0] - 2020-12-18
On hardware, just received IC TM1809 for driving the RGB led of the real alarmclock. Update the software with appropriate library [FastLED](https://github.com/FastLED/FastLED) and the MQTT led command from original project [ESParkle](https://github.com/CosmicMac/ESParkle).

### Changed
- On the hardware :
    - the RGB leds used are now those on the real alarmclock and they are now driven with the IC TM1809.
- On alarmclock MCU, `/alarmclock_ESparkle`:
    - The LedController custom class is replaced by the [FastLED](https://github.com/FastLED/FastLED) library, configured to drive a single RGB led driven by the IC TM1809.
    - The MQTT command for the led are now those from the original project [ESParkle](https://github.com/CosmicMac/ESParkle).
- On NodeRed and associated scripts, `/node_red_script`:
    - Update MQTT led command to those of the original project of ESParkle
    - Add Replay music on MP3stop or MQTT reconnect. 
