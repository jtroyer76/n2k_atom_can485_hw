# Bring-up test firmware

Checks a freshly built board with no external wiring. It repeats every 5 s and reports over USB serial (115200):

- idle levels on CAN RX (GPIO8) and RS485 RX (GPIO2)
- **CAN:** the TWAI controller in no-ACK self-test mode at 250 kbit/s. It reads its own frames back through the transceiver (TX GPIO7 → TJA1051 → RX GPIO8), so a pass shows the transceiver path works.
- **RS485:** driver enabled (DE, GPIO4), a test line sent on UART1 TX (GPIO1) at 38400. The receiver is always on (~RE tied low), so the line should echo back on RX (GPIO2).

The user LED blinks once per round if both pass, three times if anything fails.

Both transceivers run from the board's 5 V rail, which is powered only from the NMEA 2000 terminal (NET-S/NET-C), not from USB. With USB alone, expect CAN RX to sit low and both tests to fail.

```
pio run -t upload --upload-port COMx
pio device monitor -p COMx
```

No buttons needed: the board uses USB-Serial/JTAG mode, so esptool resets the XIAO into its bootloader over USB.

## Results

| Date | Board | Power | CAN | RS485 | Notes |
|---|---|---|---|---|---|
| 2026-11-17 | v0.1 #1 | USB only | fail (RX stuck low) | fail (no echo) | Expected without 12 V. Flashes over USB without buttons. |
