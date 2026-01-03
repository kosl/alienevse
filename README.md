# ESPHome Alien EVSE
[![GitHub Activity][commits-shield]][commits]
[![Last Commit][last-commit-shield]][commits]
[![Platform][platform-shield]](https://github.com/esphome)

This project [kosl/alienevse](https://github.com/kosl/alienevse) lets you use an ESP32 device to manage charging any vehicle with [ESPHome](https://esphome.io). 
It requires a simple hardware modification of that can be added to any commercial charging station and provides ESPHome EVSE addon that can control charging by dynamically control charging current while charging and schedule charging availability when at car connected. With [Home Assistant](https://www.home-assistant.io) automation it can follow available power from solar inverter or to follow grid peak shaving, multiple cars power balancing and other tarif requirements.
 
| Controls | Sensors | Diagnostic |
| - | - | - |
| <img src="./docs/ha-controls.png"> | <img src="./docs/ha-sensors.png"> | <img src="./docs/ha-diagnostic.png"> |

## Theory of operation
According to EN 61851-1 standard the theory of charger (EVSE) to vehicle (EV) communication is as depicted in the following wiki figure.
<img src="./docs/standard-type2.jpg">

AlienEVSE steps in the middle of above communication and takes over control of the Contol Pilot (CP) signal.
With CP signal to the Charger it fakes vehicle being connected. AlienEVSE with CP signal to vehicle it fakes charger being connected.
<img src="./docs/control-pilot.svg">


[releases-shield]: https://img.shields.io/github/v/release/kosl/alienevse
[commits-shield]: https://img.shields.io/github/commit-activity/m/kosl/alienevse
[last-commit-shield]: https://img.shields.io/github/last-commit/kosl/alienevse
[platform-shield]: https://img.shields.io/badge/platform-ESPHome-blue

[releases]: https://github.com/kosl/alienevse/releases
[commits]: https://github.com/kosl/alienevse/commits/main
