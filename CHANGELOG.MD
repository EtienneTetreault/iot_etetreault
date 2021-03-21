# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Group changes to describe their impact on the project, as follows:
- `Added` for new features.
- `Changed` for changes in existing functionality.
- `Deprecated` for once-stable features removed in upcoming releases.
- `Removed` for deprecated features removed in this release.
- `Fixed` for any bug fixes.
- `Security` to invite users to upgrade in case of vulnerabilities.

## [Unreleased]
Futur work : Add remote shutdown of alarm throug a hacked bathroom scale, with Load Cell Amplifier HX711.

## [0.3.0] - 2021-03-21
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


## [0.2.0] - 2020-12-18
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


## [0.1.0] - 2020-12-10
First release. Work on the MCU for the alarmclock (display time, light led and play music from stream). Small work on the MCU for a remote bathroom scale, used to stop the alarmclock from ringing. Creation of NodeRed serveur to control the alarmclock logic (state machine), host the alarmclock timer/alarm, add UI control over alarm and dispatch MQTT communication.

### Added
- On alarmclock MCU, `/alarmclock_ESparkle`:
    - Class LedController to directly manage Led On/Off, timers and messages from MQTT.
    - Use of Network Time Protocol (NTP) client to display current time on TM1637Display.
    - Use library of [ESParkle](https://github.com/CosmicMac/ESParkle) to implement [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) with MQTT and future(maybe) Led controller with [FastLED](https://github.com/FastLED/FastLED).
- On remote_scale MCU, `/esp8266_remote_scale_for_alarm`:
    - Simple esp8266 who, on powering up, emit MQTT message to stop alarmclock from ringing. Used as a remote.
    - Will eventually be used in a hacked bathroom scale, to stop alarmclock from ringing on weight detection.
- On NodeRed and associated scripts, `/node_red_script`:
    - Creation of alarmclock state machine with Xstate npm package and implementation in NodeRed function node.
    - In NodeRed, main alarm/timer for clock is manage by a user-contrib node and is integrated with UI.
    - In NodeRed, creation of Dashboard/UI to armed Alarm and set its time.

### Changed
- Change LED control from AnalogWrite to DigitalWrite. Without the use of PWM, the led doesnt create noise in the speaker and their refresh speed doesnt seems the slow down the music playing from MQTT
- Change MQTT + Music manager from [mrdiy-audio-notifier](https://gitlab.com/MrDIYca/mrdiy-audio-notifier) to [ESParkle](https://github.com/CosmicMac/ESParkle). There is no lost in MQTT and freeze of esp8266, but now, the esp8266 may bust its heap / stack overflow and reboot... Acceptable...
- Full restructure of Xstate machine for alarmclock:
    - Takes full advantage of Xstate library possibilities
    - Use of nested state : "Ringing" contains substate of various alarms
    - All alarms are lists declare outside and machine just iterate
    - Xstate internal delay (the "after" property) are use for all alarms
    - Initial "Boot" state with delay to allow NodeRed time to boot
    - No more CleanUp state : all reseting in Idle
    - New MQTT topic to fit with new AlarmClock library from ESParkle
    - Interpreter's Listener "onTransition" to publish new state to NodeRed