# n2k_atom_can485_hw — Specification

Carrier board for a Seeed Studio XIAO ESP32-S3: NMEA 2000 (CAN) plus isolated RS485/RS422, powered from the NMEA 2000 drop, in its own enclosure. (The repo name dates from a first plan built on the M5Stack ATOM; see §9.)

Status: 2026-10-30, design. Claims not yet checked against a datasheet, a measurement or the hardware are marked **(verify)**.

## 1. Goals

- One board, one cable to the NMEA 2000 drop: CAN data and power.
- RS485/RS422 port for NMEA 0183 devices (for example an AIS transponder's RS422 output at 38400 baud).
- Board and enclosure designed together (3D-printed), so nothing depends on another product's case.
- Assembled at JLCPCB from stock parts.

## 2. Non-goals (v1)

- NMEA 2000 certification. Electrical practice follows the standard where it's known (bus-powered, low load), but nothing is certified.
- Isolated CAN. The board is powered from the bus, so CAN shares the bus ground (§5.2).
- Battery operation (the XIAO's battery pads are unused).

## 3. Controller: XIAO ESP32-S3

Seeed 113991114 (plain, not Sense or Plus). Facts from Seeed's wiki (https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), Seeed's KiCad project for the board (OPL_Kicad_Library, `Seeed Studio XIAO ESP32S3/`, rev v1.2) and the ESP32-S3 datasheet v2.2.

- 21 × 17.8 mm, castellated edge pads at 2.54 mm pitch, 2 rows of 7, rows 15.24 mm apart. Components on one side only.
- Bottom-side pads: VBAT, GND, JTAG (GPIO39–42), EN, USB D−/D+. The carrier keeps traces and vias out from under the module, and leaves those pads unconnected.
- ESP32-S3 has a TWAI (CAN 2.0) controller, 1 kbit/s–1 Mbit/s, routable to any GPIO through the GPIO matrix.
- **5V pin is USB VBUS directly** (no diode on the XIAO). Seeed: "You can also use this as a voltage input but you must have some sort of diode … between your external power source and this pin". The carrier feeds it through a Schottky (§5.1).
- 3V3 pin: output of the XIAO's regulator (SGM6029). Seeed's wiki says 700 mA, its schematic says 600 mA; the carrier's 3.3 V load is small either way **(verify total)**.
- User LED on GPIO21 (active low); BOOT button on GPIO0; RESET on EN. Strapping pins GPIO0, 3, 45, 46; only GPIO3 (D2) reaches an edge pin.
- JLCPCB part C20467913 (Extended). Whether JLCPCB's assembly line places it **(verify)**; otherwise hand-soldered on its castellations.

### 3.1 Pin use

| Pin | GPIO | Use |
|---|---|---|
| D0 | GPIO1 | CAN TX |
| D1 | GPIO2 | CAN RX |
| D2 | GPIO3 | unused (strapping pin) |
| D3 | GPIO4 | spare |
| D4 | GPIO5 | I2C SDA (spare header, optional display or sensor) |
| D5 | GPIO6 | I2C SCL |
| D6 | GPIO43 | UART0 TX (debug, unused) |
| D7 | GPIO44 | UART0 RX (debug, unused) |
| D8 | GPIO7 | RS485 TX |
| D9 | GPIO8 | RS485 RX |
| D10 | GPIO9 | RS485 driver enable (pulled down: receive) |
| 5V | — | 5 V in from the carrier, through a Schottky |
| 3V3 | — | 3.3 V for the transceivers' logic side |
| GND | — | ground |

RS485 stays off UART0, so ROM boot messages never reach the RS485 line. GPIO1–9 are "freely used" pins in the ESP32-S3 datasheet.

## 4. Firmware notes

- CAN through the ESP-IDF TWAI driver (NMEA2000 library with an ESP32 TWAI back end). ESP32-S3 support in the chosen back end **(verify)**.
- RS485 on UART1, TX GPIO7, RX GPIO8, DE GPIO9 (or receive-only: leave DE low).

## 5. Circuits

### 5.1 Power (NMEA 2000 drop → 5 V)

- Input from NET-S / NET-C: reverse-polarity protection, resettable fuse, TVS, then a buck converter to 5 V.
- Input rating well above the bus's maximum voltage; parts and ratings from datasheets **(verify)**.
- 5 V → Schottky → XIAO 5V pin, so the board never drives 5 V into a USB host when the XIAO is also plugged in.
- Bus load: the board's draw from the bus sets its Load Equivalency Number (LEN, one LEN = 50 mA). Measure and label it.

### 5.2 CAN

- Non-isolated 3.3 V CAN transceiver (bus-powered device: CAN shares NET-C as ground).
- ESD protection on CANH/CANL.
- 120 Ω termination by solder jumper, open by default (a drop must not be terminated).

### 5.3 RS485 / RS422

- **Isolated** transceiver with its own isolated supply. Without isolation the RS422 device's ground and the NMEA 2000 network ground would be joined through this board, creating a second ground connection for the network.
- TX/RX + driver enable for RS485; for RS422 listen-only only A/B in is used.
- Fail-safe receiver (idle bus reads as idle), or bias resistors if the part lacks it **(verify)**.
- 120 Ω termination by solder jumper, open by default.
- Isolated-side ground brought out to the terminal (pin 7, §5.4) for devices that need a reference.

### 5.4 Connector

One 7-pin 3.5 mm pluggable terminal (the board isn't limited to 24 mm wide any more):

| Pin | Signal |
|---|---|
| 1 | NET-S (+12 V) |
| 2 | NET-C (0 V) |
| 3 | CAN-H |
| 4 | CAN-L |
| 5 | RS485 A |
| 6 | RS485 B |
| 7 | RS485 isolated GND |

Plus: XIAO USB-C reachable through the enclosure for flashing; optional 4-pin I2C header (3V3, GND, SDA, SCL).

## 6. Mechanical

- Board outline set by the parts and the enclosure; target small (around 25 × 50 mm) **(to set in layout)**.
- XIAO at one end with its USB-C at the board edge; terminal at the other end.
- Enclosure: 3D-printed, parametric (OpenSCAD or CadQuery), from the PCB outline and the parts' STEP models. Openings for the terminal, USB-C and the XIAO's LED; mounting tabs.

## 7. Phases

1. Spec and parts selection (JLCPCB stock, datasheets).
2. Schematic, ERC clean.
3. Layout; DRC clean; 3D check.
4. Gerbers, BOM, placement files; order a small assembled batch.
5. Bench test: 12 V supply, CAN on a test bus, RS422 loopback; measure current draw (LEN).
6. Enclosure.

## 8. Sources

- XIAO ESP32-S3 wiki: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
- Seeed KiCad library: https://github.com/Seeed-Studio/OPL_Kicad_Library (`Seeed Studio XIAO Series Library/`, symbol `XIAO-ESP32-S3-SMD`, footprint `XIAO-ESP32-S3-SMD.kicad_mod`)
- XIAO ESP32-S3 3D model: https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/res/seeed-studio-xiao-esp32s3-3d_model.zip
- ESP32-S3 datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- JLCPCB part C20467913: https://jlcpcb.com/partdetail/Seeed-113991114/C20467913

## 9. History

First planned as an M5Stack ATOM base in the Atomic base format, combining an RS485 tail and a CAN base. Dropped because the Atomic base's PCB outline isn't published as a drawing; a XIAO carrier with its own enclosure has no such dependency.
