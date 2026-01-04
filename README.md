# ESPHome Alien EVSE
[![GitHub Activity][commits-shield]][commits]
[![Last Commit][last-commit-shield]][commits]
[![Platform][platform-shield]](https://github.com/esphome)

This project [kosl/alienevse](https://github.com/kosl/alienevse) lets you use an ESP32 device to manage charging any vehicle with [ESPHome](https://esphome.io). 
It requires a simple hardware modification of that can be added to any commercial charging station and provides ESPHome EVSE addon that can control charging by dynamically control charging current while charging and schedule charging availability when at car connected. With [Home Assistant](https://www.home-assistant.io) automation it can follow available power from solar inverter or to follow grid peak shaving, multiple cars power balancing and other tarif requirements.
 
| Controls | Sensors | Diagnostic |
| - | - | - |
| <img src="./docs/ha-controls.png"> | <img src="./docs/ha-sensors.png"> | <img src="./docs/ha-diagnostic.png"> |

## Features
- Configuration with ESPHome YAML only (see [esphome/alienevse.yaml](esphome/alienevse.yaml) as an example).
- Setting current and enabling/disabling charging.
- Sensors (optional) for current transformers (CT) can be attached inside wallbox providing parallel measurements. Instead of internal CT clamp measurements, external CTs for measuring grid currents for "solar charging" or similar strategies can be used.
- Standalone web portal (use without Home Assistant) and built in automation is in principle possible.
- Cheap DIY (under €10) hardware and easy modification of any commercial charger. Existing charger electronics is left intact. Only Control Pilot (CP) wire is split inside charger.
- Testing  of the AlienEVSE board can be done without the charger since the board provides CP signal for the vehicle and for the charger. 
- Can be used with commercial chargers for one or three phases. 

## Theory of operation
According to EN 61851-1 standard the theory of charger (EVSE) to vehicle (EV) communication is as depicted in the following figure.

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
| **E**     | CP short to PE (fault)                    |            0 V |              **N/A** | Fault                                         | Immediate shutdown                          |
    
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

Complete schematics and board are available under [schematics/](./schematics/). 
| Signal | Pin | Function | Description |
|-|-|-|-|
| CP_CH_IN | GPIO1 | (ADC) | Measuring charger CP voltage (used for debugging). R1 and R8 can be omitted! |
| CP_CH_OUT_B | GPIO7 | (digital out) | Signal to charger that EV is connected |
| CP_CH_OUT_C | GPIO6 | (digital out) | Signal to charger that EV is requesting charging |
| CP_EV_IN | GPIO0 | (ADC) |  Measuring raw (including PWM) voltage at CP connected to EV |
| CP_EV_OUT | GPIO10 | (PWM out) | Generating PWM signal for EV that is amplified to +- 12 V |
| CC_L1 | GPIO2 | (ADC) | Current clamp (CC) input (optional) for measuring Line 1 current |
| CC_L2 | GPIO3 | (ADC) | Current clamp (CC) input (optional) for measuring Line 2 current |
| CC_L3 | GPIO4 | (ADC) | Current clamp (CC) input (optional) for measuring Line 3 current |

### What Charger?


What charger can be used for this modification. We recommend the cheapest one without WiFi that has required power rating and safety protection including display showing measured current. Some shown below can be found for under €130 for 3 phase 11 kW Type 2 charger that is most comonly used in EU cars.   

| Aandaiic | Feyree | Kolanky |
|-|-|-|
|<img src="./docs/charger-andaiic.png"> | <img src="./docs/charger-feyree.png"> | <img src="./docs/charger-kolanky.png"> |

### Locating connections points inside a charger

There are several wires (up to 9) wires required to be soldered to existing charger. Most of them can be spotted by observing the PCB of the charger. 
The easiest in to spot tiny CP wire coming from the PCB to the EV charging cable. That CP wire is to be disconnected and AlienEVSE CP control wil be put in the middle. 
Note that usualy there is no Proximity Pilot (PP) wire going to the plug as the resistor rescribing cable strength is  installed in the handle directly to save on costs of the PP wire.  
| Wire name | Location | 
|-----------|----------|
| GND   | Ground is the largest area on the PCB being also around the PCB mounting holes |
| +12 V | Look for markings +12 under PCB or measure around CP amplifier or around linear regulators |
| -12 V | Similarly to +12 the voltage should be quite stable.* |
| +5 V  | Using 5V for powering ESP32 is recommended.* |
| +3.3V | Alternatively, +3.3 V can be used for powering ESP32.*  |
| CP    | Going to the charging cable. Break this wire and connect CP_CH and CP_EV signals in between |
| CC_L1 | If there are current transformers (CT have usually ratio 1000:1) connect to the live wire.** |
| CC_L2 | As with CC_L1 the CT can be attached to charger's sensor in paralle without affecting measurements.** |
| CC_L3 | All CC lines can be omitted or connected to external CTs for measuring complete grid |

*Note that 100 nF blocking capatitors are required on AlienEVSE board to block interference.

**Live wire can be found with multimeter when charging. There will be some positive (DC) offset to allow negative CT current. 
When measuting AC voltage (200 mV to 2V range) on one or another wire (eg. red or black) then one will show some AC voltage while another will be zero. Use that show some AC voltage proportional to charging current. 

### Testing

Prototype board can be easily tested without EV and charger provided +12 V and -12 V voltages for supply, while boarc can be powered though USB used for programming the ESP32-C3 module. Tor testing connect CP_CH and CP_EV wires together and simulate EV states by `CP_CH_OUT_B` and `CP_CH_OUT_C` signals. Note that charging logic inside `evse_update_state_script` needs to return early without changing `CP_CH_OUT_B` and `CP_CH_OUT_C` signals at the end of the `evse_update_state_script`.

## ESPHome software

Use [alienevse.yaml](./esphome/alienevse.yaml) as an example and modify ranges for your case. Burning firmware can be done through Home Asistant ESPHome builder or from a desktop with a Python using
~~~ bash
python -m venv install local
local/bin/pip install esphome
local/bin/esphome compile alienevse.yaml
local/bin/esphome upload alienevse.yaml
~~~


[releases-shield]: https://img.shields.io/github/v/release/kosl/alienevse
[commits-shield]: https://img.shields.io/github/commit-activity/m/kosl/alienevse
[last-commit-shield]: https://img.shields.io/github/last-commit/kosl/alienevse
[platform-shield]: https://img.shields.io/badge/platform-ESPHome-blue

[releases]: https://github.com/kosl/alienevse/releases
[commits]: https://github.com/kosl/alienevse/commits/main
