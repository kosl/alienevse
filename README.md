# ESPHome Alien EVSE
[![GitHub Release][releases-shield]][releases]
[![GitHub Activity][commits-shield]][commits]
[![Last Commit][last-commit-shield]][commits]
[![Platform][platform-shield]](https://github.com/esphome)

This project [kosl/alenevse](https://github.com/kosl/alienevse) lets you use an ESP32 device to manage charging any vehicle with ESPHome. 
It requires a simple hardware modification of that can be added to any commercial charging station and provides ESPHome EVSE addon that can control charging by dynamically control charging current while charging and schedule charging availability when at car connected. With [Home Assistant](https://www.home-assistant.io) automation it can follow available power from solar inverter or to follow grid peak shaving, multiple cars power balancing and other tarif requirements.
 
| Controls | Sensors | Diagnostic |
| - | - | - | - |
| <img src="./docs/ha-controls.png"> | <img src="./docs/ha-sensors.png"> | <img src="./docs/ha-diagnostic.png"> |

