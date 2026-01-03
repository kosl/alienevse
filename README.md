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
With CP signal to the Charger it fakes vehicle being connected. AlienEVSE with CP signal to the vehicle (EV) it fakes charger being connected.

<img src="./docs/control-pilot.svg">

The CP signal is the only thing needed to communicate with EV and to control charging current with pulse width modulation (PWM) that operates at frequency of 1kHz.
To generate CP for vehigle and to signal CP to Charger/Wallbox a processor is required with only few additional analog circuits to generate signals ranging from +12 V to -12 V. 

| CP State  | Name / Meaning                            | CP Voltage (≈) |       PWM Duty Cycle | What PWM Means                                | EVSE / EV Action                            |
| --------- | ----------------------------------------- | -------------: | -------------------: | --------------------------------------------- | ------------------------------------------- |
| **A**     | No EV connected                           |       +12 V DC | **N/A** (or 100% DC) | No current advertised                         | EVSE idle, contactors open                  |
| **B**     | EV connected, not charging                |           +9 V |     **≥10% to ≤96%** | Advertised **max current** available          | EV detected; EVSE ready; ISO: SLAC/TLS here |
| **C**     | Charging requested (no ventilation)       |           +6 V |     **≥10% to ≤96%** | Advertised **max current**                    | EVSE closes contactors; charging allowed    |
| **D**     | Charging requested (ventilation required) |           +3 V |     **≥10% to ≤96%** | Advertised **max current** (with ventilation) | Charge only if ventilation supported        |
| **E**     | CP short to PE (fault)                    |            0 V |              **N/A** | Fault                                         | Immediate shutdown                          |
| **F**     | CP fault / invalid                        |        Invalid |              **N/A** | Fault                                         | Charging prohibited                         |
| **CPoff** | CP disabled (not an IEC state)            |     Hi-Z / off |              **N/A** | EVSE inactive / emergency                     | Use only for safety/power loss              |

PWM Duty Cycle → Maximum Current (IEC 61851)
PWM frequency: 1 kHz
Duty cycle is valid only in States B, C, D

| Duty Cycle (%) | Max Current Advertised             |
| -------------: | ---------------------------------- |
|         10–85% | **I (A) = DutyCycle × 0.6**        |
|         86–96% | **I (A) = (DutyCycle − 64) × 2.5** |
|      100% (DC) | Level comm only (no PWM signal)    |
|           <10% | Not valid / no charging            |


## Implementation
For CP signaling the cheapest ESP32-C3 module (€2.5-€4) and some resistors, 2 transistors and operational amplifier are required.
Compete cost is under €10. It can be protyped on a thru-hole board of 3x5 cm dimension as shown below.
| AlienEVSE board with ESP32-C3 supermini | Installed under cover | Wallbox |
| - | - | - |
| <img src="./photos/20260103_142043.jpg"> | <img src="./photos/20260103_151713.jpg"> | <img src="./photos/20260103_151722.jpg"> |

### Schematics
| CPU (ESP32-C3 supermini) | Charger CP control | EV CP control |
| - | - | - |
| <img src="./docs/schematics-cpu.png"> | <img src="./docs/schematics-cp-charger.png"> | <img src="./docs/schematics-cp-ev.png"> |


[releases-shield]: https://img.shields.io/github/v/release/kosl/alienevse
[commits-shield]: https://img.shields.io/github/commit-activity/m/kosl/alienevse
[last-commit-shield]: https://img.shields.io/github/last-commit/kosl/alienevse
[platform-shield]: https://img.shields.io/badge/platform-ESPHome-blue

[releases]: https://github.com/kosl/alienevse/releases
[commits]: https://github.com/kosl/alienevse/commits/main
