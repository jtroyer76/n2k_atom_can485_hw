# n2k_atom_can485_hw — Specification

Carrier board for a Seeed Studio XIAO ESP32-S3: NMEA 2000 (CAN) plus isolated RS485/RS422, powered from the NMEA 2000 drop, in its own enclosure. (The repo name dates from a first plan built on the M5Stack ATOM; see §9.)

Status: 2026-10-31, schematic (ERC clean) and PCB layout v0.1 (DRC: no errors; cosmetic silkscreen and thermal-relief warnings only) done; fab files in `fab/`; not yet ordered or tested. Claims not yet checked against a datasheet, a measurement or the hardware are marked **(verify)**.

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
- JLCPCB part C20467913 (Extended). Hand-soldered on its castellations, not placed by JLCPCB: it uses `n2k_atom_can485:XIAO-ESP32-S3-SMD-HandSolder`, Seeed's footprint with the paste layer removed, so the stencil leaves no solder bumps under the module.

### 3.1 Pin use

| Pin | GPIO | Use |
|---|---|---|
| D0 | GPIO1 | RS485 TX |
| D1 | GPIO2 | RS485 RX |
| D2 | GPIO3 | unused (strapping pin) |
| D3 | GPIO4 | RS485 driver enable (pulled down: receive) |
| D4 | GPIO5 | spare |
| D5 | GPIO6 | spare |
| D6 | GPIO43 | UART0 TX (debug, unused) |
| D7 | GPIO44 | UART0 RX (debug, unused) |
| D8 | GPIO7 | CAN TX |
| D9 | GPIO8 | CAN RX |
| D10 | GPIO9 | spare |
| 5V | — | 5 V in from the carrier, through a Schottky |
| 3V3 | — | 3.3 V for the transceivers' logic side |
| GND | — | ground |

RS485 stays off UART0, so ROM boot messages never reach the RS485 line. GPIO1–9 are "freely used" pins in the ESP32-S3 datasheet. The split follows the layout: with the USB-C at the board edge, D0–D6 face the RS485 side of the board and D7–D10 the CAN side.

## 4. Firmware notes

- CAN through the ESP-IDF TWAI driver (NMEA2000 library with an ESP32 TWAI back end). ESP32-S3 support in the chosen back end **(verify)**.
- CAN: TX GPIO7, RX GPIO8.
- RS485 on UART1: TX GPIO1, RX GPIO2, DE GPIO4 (or receive-only: leave DE low).

## 5. Circuits

### 5.1 Power (NMEA 2000 drop → 5 V)

- Input from NET-S / NET-C: reverse-polarity protection, resettable fuse, TVS, then a buck converter to 5 V.
- Input rating well above the bus's maximum voltage; parts and ratings from datasheets **(verify)**.
- 5 V → Schottky → XIAO 5V pin, so the board never drives 5 V into a USB host when the XIAO is also plugged in.
- Bus load: the board's draw from the bus sets its Load Equivalency Number (LEN, one LEN = 50 mA). Measure and label it.

### 5.2 CAN

- Non-isolated CAN transceiver, 5 V supply with 3.3 V logic (bus-powered device: CAN shares NET-C as ground).
- ESD protection on CANH/CANL.
- 120 Ω termination by solder jumper, open by default (a drop must not be terminated).

### 5.3 RS485 / RS422

- **Isolated** transceiver with its own isolated supply. Without isolation the RS422 device's ground and the NMEA 2000 network ground would be joined through this board, creating a second ground connection for the network.
- TX/RX + driver enable for RS485; for RS422 listen-only only A/B in is used.
- Supplies: VCC from the 5 V rail (feeds the isolated DC-DC), VCCL (logic) from the XIAO's 3.3 V. SEL tied to VISO: VISO = 5 V. ~RE tied low: receiver always on. DE has a 10 k pull-down so the driver stays off during reset.
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

Plus: XIAO USB-C reachable through the enclosure for flashing. No other connectors.

Header MAX MX15EDGRC-3.5-07P (LCSC C5188300), mating plug MX15EDGK-3.5-07P (C5188254). Footprint drawn for it (`kicad/lib/n2k_atom_can485.pretty`): Ø1.4 mm holes, 3.5 mm pitch, body 25.4 × 9.2 mm, from the family drawing MX15EDGR-3.5-XXP (the 7-pin datasheet itself wasn't downloadable) **(verify against a part)**. Pin labels are on the back silkscreen. Pins 4 (CAN-L) and 5 (RS485 A) are 3.5 mm apart, about 1.3 mm of copper between pads: the isolation is functional (ground loop), not a safety barrier.

### 5.5 Parts (chosen 2026-10-30)

Specs from each datasheet; stock and prices from JLCPCB/LCSC on 2026-10-30 (recheck at order time). Lib: B = Basic, PE = Preferred Extended, E = Extended.

| Function | Part | LCSC | Lib | Key specs (datasheet) |
|---|---|---|---|---|
| Buck 12 V → 5 V | TI LMR54406DBVR, SOT-23-6 | C5219316 | E | VIN 4–36 V, abs max 45 V; 0.6 A; IQ 80 µA typ (PFM variant); VREF 0.8 V. Table 8-1 (5 V): L 15 µH, COUT 22 µF/10 V, RFBT 118 k / RFBB 22.1 k. Built with 120 k / 22 k (Basic parts): 0.8 × (1 + 120/22) = 5.16 V, under the transceivers' 5.5 V maximum |
| Buck inductor | Sunlord SWPA5040S150MT, 15 µH, 5×5 mm | C44888 | E | Isat 2.0 A, DCR 0.112 Ω max |
| Input fuse | BHFUSE BSMD1812-050-60V, 1812 PTC | C883142 | E | Ihold 0.5 A, Itrip 1.0 A, Vmax 60 V |
| Input TVS | SMBJ20A (Hongjiacheng) | C19077575 | PE | VRWM 20 V, VBR 22.2–24.5 V, VC 32.4 V @ 18.5 A (below the buck's 45 V abs max) |
| Reverse polarity | MDD SS14, SMA Schottky | C2480 | B | 40 V, 1 A, VF 0.55 V @ 1 A |
| 5 V → XIAO diode | JSCJ B5819W, SOD-123 | C8598 | B | 40 V, 1 A, VF 0.6 V max @ 1 A |
| CAN transceiver | NXP TJA1051T/3/1J, SOIC-8 | C38695 | E | VCC 4.5–5.5 V (from the 5 V rail), VIO 2.8–5.5 V (3.3 V logic); CANH/CANL ±58 V; pin 8 S has an internal pull-down (Normal mode) — tied to GND |
| CAN ESD | Nexperia PESD2CAN,215, SOT-23 | C75176 | E | VRWM 24 V, 25 pF typ, IEC 61000-4-2 > 8 kV |
| Isolated RS485 | Chipanalog CA-IS2092VW, SOIC-16 WB | C7422559 | E | 3.75 kVrms isolation; integrated isolated DC-DC; VCC 3.15–5.5 V (run at 5 V, VISO 5 V), VCCL logic supply 3.3 V; 0.5 Mbps half-duplex; true fail-safe receiver; ICC at 3.3 V 17 mA typ unloaded, 88 mA max with 120 Ω |
| Terminations | UNI-ROYAL 0603WAF1200T5E, 120 Ω 1 % | C22787 | B | via solder jumpers, open by default |
| Terminal block | MAX MX15EDGRC-3.5-07P-GN01-Cu-A header, right angle | C5188300 | E | 7 pins, 3.5 mm; plug MX15EDGK-3.5-07P (C5188254). 300 V / 8 A per the family drawing |
| Passives | 0603/0805/1206 MLCCs and 1 % 0603 resistors | C29823, C14663, C45783, C15850, C15849, C25808, C31850, C25804, C22787 | B | values and LCSC numbers in the schematic and `fab/` BOM. C25808 (120 k) checked only against a third-party copy of JLCPCB's database **(verify)** |

Choices:
- **TJA1051T/3 over SN65HVD230:** the board has a 5 V rail, so the 5 V-supply part with a 3.3 V logic pin works, and its bus pins take ±58 V against the SN65HVD230's −4 to +16 V. A NET-S (12 V) wire landing on CAN-H, a likely wiring mistake, is survivable.
- **LMR54406 over TPS560430 / LMR50410:** 45 V abs max leaves margin over the TVS clamp (32.4 V); the other two are 38 V abs max.
- **CA-IS2092VW over ADM2587E:** higher isolation (3.75 vs 2.5 kVrms), smaller package, about a third of the price.
- **3.3 V budget** on the XIAO's regulator (600–700 mA per Seeed): ESP32-S3 with WiFi, plus the TJA1051 VIO and CA-IS2092VW VCCL logic pins. The transceivers' supply current comes from the 5 V rail.
- **5 V budget** (buck, 0.6 A): XIAO via the Schottky, TJA1051 VCC, CA-IS2092VW VCC (≤ 88 mA max at 3.3 V supply with 120 Ω; its 5 V figure **(verify)**).
- No Basic or Preferred part exists for the buck, inductor, PTC, CAN ESD, isolated RS485 or terminal, so the build pays JLCPCB's Extended-part fees.

## 6. Mechanical

- Board 32 × 64 mm, 1 mm corner radius, 2 layers, 1.6 mm. Terminal on one short edge (plug side flush with the edge), XIAO on the other with its USB-C at the edge.
- No mounting holes: every place they fit would block the routing channels beside the XIAO. The enclosure holds the board by its edges (slots or a clamped lid).
- Isolation: the RS485 side (terminal pins 5–7, U3's bus side, its capacitors and termination) has its own GNDB pour on both layers, separated from GND by 1.5 mm; tracks and vias of non-isolated nets stay 1 mm clear of it. A custom DRC rule (`kicad/n2k_atom_can485.kicad_dru`, `isolation_gap`) enforces the 1 mm, so every DRC run checks it.
- No copper under the XIAO (rule area): its bottom pads (JTAG, USB, battery) land on unconnected pads from Seeed's footprint (no paste on them, see §3).
- Layout notes: the B.Cu layer is a near-solid ground plane; every SMD ground pad has a via to it. Track widths 0.5 mm for power, 0.4 mm for 3.3 V and VISO, 0.25 mm for signals; vias 0.6/0.3 mm. Routed by a script (grid maze router). Buck layout follows the LMR54406 datasheet (SLUSEG8E §8.4, Figure 8-11). C2 (100 nF) sits just below U1: its VIN pad is wired straight up into the VIN and EN pins (both VIN), and its GND pad returns to U1's GND pin by a top-layer track under the package, between the FB and EN pins, with a via to the B.Cu plane there. The input loop is about 2 × 2 mm. C1 (4.7 µF) sits below C2 on the same two tracks. The boot cap C3 is beside CB/SW, SW to L1 is short, and the FB resistors are beside FB. Check SW ringing and output ripple with a scope at bring-up.
- Routing cross-check (v0.1): the same placement was autorouted with Freerouting 2.5.0, with only B.Cu given as the ground plane, and passed DRC and schematic parity. The script routing was kept. It puts a ground via 0.3–0.5 mm from every capacitor's ground pad; Freerouting's nearest vias were 1.1–8.5 mm away, with ground run as tracks on F.Cu. Freerouting's only advantage was neater 45° pad entries. The script routing had 110 short off-angle jogs from the routing grid to off-grid pad centres. Each lay wholly inside its pad, so it was invisible in the copper. These and other in-pad stubs (138 segments) are now removed, so every track ends on the grid inside its pad.
- Enclosure: 3D-printed, parametric (OpenSCAD or CadQuery), from the PCB outline and the parts' STEP models. Openings for the terminal, USB-C and the XIAO's LED; mounting tabs.

## 7. Phases

1. Spec and parts selection (JLCPCB stock, datasheets).
2. Schematic, ERC clean.
3. Layout; DRC clean; 3D check. **Done (v0.1).**
4. Gerbers, BOM, placement files (`fab/`); order a small assembled batch. J1 (terminal) and U4 (XIAO) are hand-soldered and left out of the JLCPCB BOM and placement files; JLCPCB stocks both, but its preview placed them wrongly and both are easy by hand. Before ordering, check part rotations in JLCPCB's placement preview (SOT-23, SOIC and diode footprints often need a rotation offset).
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
