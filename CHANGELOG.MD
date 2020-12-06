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
### Added
- Class LedController to manage Led On/Off, timers and messages from MQTT.
- Creation of alarmclock state machine with Xstate npm package and implementation in NodeRed function node.
- In NodeRed, main alarm/timer for clock is manage by a user-contrib node and is integrated with UI.

### Changed
- Change LED control from AnalogWrite to DigitalWrite. Without the use of PWM, the led doesnt create noise in the speaker et their refresh speed doesnt seems the slow down the music playing from MQTT


## [0.0.1] - 2017-06-20
<!-- Edit first release and release date -->
### Added