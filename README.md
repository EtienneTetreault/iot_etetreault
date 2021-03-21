# iot_etetreault
Repository for all personnal IoT related script, specifically embedded microcontroler source files.

## Current release [0.3.0] - 2021-03-21
On hardware, just received DAC+Ampli breakout board MAX98357A, that takes the I2S music signal from the ESP8266 and output to the speaker. Sound is a lot better :) Took to occasion of re-wiring to migrate all the project from Wemos D1 to Wemos Mini (both based on ESP8266), which is smaller and will be used for the final, soldered protoboard. Also, on NodeRed, some cool update to time management, time zone and time calculation.

### Added
- On NodeRed :
    - UTC Time Zone set by user and used by Node Red alarm and alarmclock_ESparkle time display
    - Sleep time predicted shown as notification when arming the alarm
- On NodeRed associated scripts, `/node_red_script`:
    - Create state after the final snoozing and before reset, where the light is up to help in the dark morning

### Changed
- On the hardware :
    - integration of MAX98357A and use of I2S signal from the ESP8266
    - main MCU is now Wemos Mini instead of Wemos D1 (both based on ESP8266)
- On alarmclock MCU, `/alarmclock_ESparkle`:
    - Use predefined settings from class ESP8266Audio to output sound signal as I2S to the new DAC+Ampli
    - At MQTT connection (boot), fetch over MQTT and use Node Red user TimeZone to update the NTPC time client


## Futur Section : Schematic and Wiring
- Explain and show proof of class AudioOutput.SetGain with limit value 0 to 4. Makes some test of most confortable sound!!
- Show schematic for I2S pinout of ESP-8266 Mini from ESParcle project
- Explain add item to schematic :
    - Resistor to boost speaker impedance (is 3 Ohm and Adafruit recommand >4 Ohm for ampli MAX98357A)
    - Resistor to diminish the natural gain in dB of ampli MAX98357A