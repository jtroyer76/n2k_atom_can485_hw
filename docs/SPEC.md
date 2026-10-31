# n2k_atom_can485_hw — Specification

Base board for the M5Stack ATOM: NMEA 2000 (CAN) plus RS485/RS422, powered from the NMEA 2000 drop, in the Atomic base format.

Status: 2026-10-30, design. Claims not yet checked against a datasheet, a measurement or the hardware are marked **(verify)**.

## 1. Goals

- One ATOM, one board, one cable to the NMEA 2000 drop: CAN data and power.
- RS485/RS422 port for NMEA 0183 devices (for example an AIS transponder's RS422 output at 38400 baud).
- Fits the Atomic base case footprint (24 × 48 mm) so a stock Atomic case or a printed copy can be used.
- Assembled at JLCPCB from stock parts.

## 2. Non-goals (v1)

- NMEA 2000 certification. Electrical practice follows the standard where it's known (bus-powered, low load), but nothing is certified.
- Isolated CAN. The board is powered from the bus, so CAN shares the bus ground (§5.2).
- Other ATOM models' pin mappings beyond the ATOM Lite until checked (§4).

## 3. Reference designs (facts, for context)

From M5Stack's product pages and schematic images, read 2026-10-30.

**Atomic CAN Base (A103):**
- 24 × 48 × 18 mm, PC case. Mechanical drawing: 24 × 48 × 17.5 mm overall; 3.96 mm 4-pin terminal on the short end; keyhole Ø3.2 and two holes 16 mm apart on the back.
- CAN transceiver CA-IS3050G (isolated), powered on the bus side by a B0505S-1W isolated 5 V → 5 V converter.
- **Powered from the ATOM** (its 5 V pin), not from the bus. Terminal J1: CANH, CANL, NC, GND (to shield).
- ATOM headers: P1 (5-pin): pin 1 3.3 V, pin 2 CAN_TX, pin 3 CAN_RX, pins 4–5 unused. P2 (4-pin): pins 1–2 unused, pin 3 5 V, pin 4 GND.
- Bus protection: 100 mA 60 V PTC fuse on each of CANH and CANL, TSS and GDT surge parts, ESD diodes.

**Tail485 (T002):**
- SP485EEN-L transceiver; AOZ1282CI buck, 9–24 V → 5 V, feeding the ATOM through its USB-C 5 V input.
- Connects to the ATOM's Grove port (GND, 5 V, TXD, RXD; G26/G32 on the ATOM Lite). 54 × 96 × 10 mm, separate from the ATOM.

## 4. ATOM interface

Same two headers as the Atomic bases (P1 5-pin, P2 4-pin). Header positions on the PCB come from measuring a stock base **(verify)**.

| Header pin | Use | ATOM Lite GPIO |
|---|---|---|
| P1-1 | 3.3 V (logic supply for transceivers, from the ATOM) | — |
| P1-2 | CAN TX | G22 **(verify)** |
| P1-3 | CAN RX | G19 **(verify)** |
| P1-4 | RS485 TX | G23 **(verify)** |
| P1-5 | RS485 RX | G33 **(verify)** (input-capable is enough) |
| P2-1 | RS485 driver enable (pulled to receive) | G21 **(verify)** |
| P2-2 | free | G25 **(verify)** |
| P2-3 | 5 V to the ATOM | — |
| P2-4 | GND | — |

CAN TX/RX use the same header pins as the Atomic CAN Base, so CAN firmware written for it works unchanged. The ATOM Lite GPIO numbers and the AtomS3 / AtomS3 Lite mapping still need the ATOM pinout docs **(verify)**.

## 5. Circuits

### 5.1 Power (NMEA 2000 drop → 5 V)

- Input from NET-S / NET-C: reverse-polarity protection, resettable fuse, TVS, then a buck converter to 5 V.
- Input rating well above the bus's maximum voltage; part choice and ratings from datasheets **(verify)**.
- 5 V goes to the ATOM's 5 V pin through a Schottky or ideal diode, so the board never drives 5 V back into a USB host when the ATOM is also plugged in. Whether the ATOM's 5 V pin connects straight to USB VBUS **(verify)**.
- Bus load: the board's current draw from the bus sets its Load Equivalency Number (LEN, one LEN = 50 mA). Measure and label it.

### 5.2 CAN

- Non-isolated 3.3 V CAN transceiver (bus-powered device: CAN shares NET-C as ground).
- ESD/surge protection on CANH/CANL.
- 120 Ω termination by solder jumper, open by default (a drop must not be terminated).
- Status LEDs optional (space permitting).

### 5.3 RS485 / RS422

- **Isolated** transceiver with its own isolated supply, powered from the 3.3 V or 5 V side. Without isolation the RS422 device's ground and the NMEA 2000 network ground would be joined through this board, creating a second ground connection for the network.
- Full use for RS485 (TX/RX + driver enable); for RS422 listen-only, only A/B in is needed.
- Fail-safe receiver (idle bus reads as idle), or bias resistors if the part lacks it **(verify)**.
- 120 Ω termination by solder jumper, open by default.
- Whether the isolated side's ground needs a terminal pin for any common RS422 device **(verify)**; v1 has none (§5.4).

### 5.4 Connector

One 6-pin 3.5 mm pluggable terminal on the short end (two 3.96 mm terminals don't fit across 24 mm):

| Pin | Signal |
|---|---|
| 1 | NET-S (+12 V) |
| 2 | NET-C (0 V) |
| 3 | CAN-H |
| 4 | CAN-L |
| 5 | RS485 A |
| 6 | RS485 B |

Pin order and whether a 3.5 mm pluggable terminal's height fits under the case lid **(verify)**.

## 6. Mechanical

- Board outline, thickness, mounting points and header positions taken from a stock Atomic base PCB **(verify — to measure)**.
- Case: a stock Atomic case with the terminal opening widened, or a 3D-printed case to the same outline.

## 7. Phases

1. Spec and parts selection (JLCPCB stock, datasheets).
2. Schematic, ERC clean.
3. Board outline from measurements; layout; DRC clean; 3D check against the case.
4. Gerbers, BOM, placement files; order a small assembled batch.
5. Bench test: power from a 12 V supply, CAN on a test bus, RS422 loopback; measure current draw (LEN).
6. Case.

## 8. Sources

- Atomic CAN Base: https://docs.m5stack.com/en/atom/Atomic_CAN_Base (schematic image, mechanical drawing, CA-IS3050G datasheet link)
- Tail485: https://docs.m5stack.com/en/atom/tail485 (block diagram, SP485EEN and AOZ1282CI datasheet links)
