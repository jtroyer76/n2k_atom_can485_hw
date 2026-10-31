# n2k_atom_can485_hw

A base board for the M5Stack ATOM that puts NMEA 2000 (CAN) and RS485/RS422 on one board, in the same 24 × 48 mm Atomic base format. It is powered from the NMEA 2000 drop, so one cable carries power and data. Made for gateways between NMEA 0183 devices on RS422 (for example an AIS transponder) and an NMEA 2000 network.

Status: design. No schematic or PCB yet. Design and open questions: [docs/SPEC.md](docs/SPEC.md).

## Planned features

- NMEA 2000 / CAN transceiver, 3.3 V logic, on the ATOM's CAN pins.
- Isolated RS485/RS422 transceiver, so the RS422 device's ground stays separate from the NMEA 2000 network ground.
- 12 V NMEA 2000 network power → 5 V for the ATOM, with reverse-polarity, overcurrent and surge protection.
- Optional 120 Ω termination on each bus, off by default.
- One pluggable terminal block: NET-S, NET-C, CAN-H, CAN-L, RS485 A, RS485 B.
- Built at JLCPCB with SMT assembly; Gerbers, BOM and placement files in releases.

## License

CERN-OHL-S-2.0. See [LICENSE](LICENSE).
