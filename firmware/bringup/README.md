# Bring-up test firmware

Checks a freshly built board with no external wiring. It repeats every 5 s and reports over USB serial (115200):

- idle levels on CAN RX (GPIO8) and RS485 RX (GPIO2)
- **CAN DC:** CAN TX driven as a plain GPIO, CAN RX read with a pull-down. RX must go low when TX is low and be driven high when TX is high. The pull-down matters: without it an unsoldered RX joint floats high and looks like a healthy recessive bus.
- **CAN:** the TWAI controller in no-ACK self-test mode at 250 kbit/s. It reads its own frames back through the transceiver (TX GPIO7 → TJA1051 → RX GPIO8), so a pass shows the transceiver path works.
- **RS485:** driver enabled (DE, GPIO4), a test line sent on UART1 TX (GPIO1) at 38400. The receiver is always on (~RE tied low), so the line should echo back on RX (GPIO2).

The user LED blinks once per round if both pass, three times if anything fails.

Both transceivers run from the board's 5 V rail, which is powered only from the NMEA 2000 terminal (NET-S/NET-C), not from USB. With USB alone, expect both tests to fail.

```
pio run -t upload --upload-port COMx
pio device monitor -p COMx
```

No buttons needed: the board uses USB-Serial/JTAG mode, so esptool resets the XIAO into its bootloader over USB.

## Meter mode: `xiao-txd-square`

`pio run -e xiao-txd-square -t upload` runs a ~10 kHz 50 % software square wave on CAN TX, with no CAN controller involved. Each low lasts 50 µs, well under the TJA1051's 0.3 ms minimum TXD dominant time-out (datasheet `tto(dom)TXD`), so the transmitter keeps driving. A DC meter on U2's legs then reads averages: pin 1 (TXD) about VIO/2, pin 7 (CANH) above and pin 6 (CANL) below the 2.6 V recessive level if the transmitter drives, and pin 4 (RXD) about VIO/2 if the receiver follows. Each second it also prints whether the TX pad and RX follow, with the RX pull cycled up, down and none: a connected RXD toggles under all three, while an open RX line follows the pull.

## Results

| Date | Board | Power | CAN | RS485 | Notes |
|---|---|---|---|---|---|
| 2026-11-17 | v0.1 #1 | USB only | fail | fail (no echo) | Expected without 12 V. Flashes over USB without buttons. (D9 was already open here; see below.) |
| 2026-11-17 | v0.1 #1 | 12.85 V + USB | fail | pass (24/24) | CAN RX open: XIAO D9 castellation not soldered. U2 measured VCC 5.18 V, VIO 3.28 V, S 0 V, CANH/CANL 2.6 V recessive. In meter mode TXD averaged 1.665 V, CANH 3.57 V and CANL 1.65 V (transmitter driving), but GPIO8 followed its internal pull. |
| 2026-11-17 | v0.1 #1 | 12.85 V + USB | **pass** (DC; 10/10 frames, TEC/REC 0, 0 bus errors) | **pass** (24/24) | After reflowing D9. |
