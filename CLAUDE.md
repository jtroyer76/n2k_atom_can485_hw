# n2k_atom_can485_hw

Carrier board for the Seeed Studio XIAO ESP32-S3: NMEA 2000 (CAN) and RS485/RS422, powered from the NMEA 2000 drop, with a 3D-printed enclosure. Start with [README.md](README.md); the design is in [docs/SPEC.md](docs/SPEC.md).

## Rules

- **Commits straight to `main` until the first release tag;** after that, pull requests only (branch from `main`, one topic per PR, CI must pass; the assistant opens PRs, the owner reviews and merges, the assistant never approves, merges or pushes to `main`).
- **Conventional Commits:** `type: summary` (`feat`, `fix`, `docs`, `test`, `ci`, `build`, `refactor`, `perf`, `chore`). PRs are squash-merged, so the PR title is the commit on `main`.
- **KiCad 10.** Design files are saved with KiCad 10; they don't open in older versions. DRC must pass before Gerbers are exported.
- **Parts from JLCPCB stock** for assembled builds: every part has an LCSC number in the BOM, checked against the datasheet.
- **Standalone product.** Don't reference the owner's other repos or projects.
- **Stay general.** Public product: no boat names, hostnames or boat-specific details in docs, examples or photos. "Tested on a boat" is fine.
- **No AI attribution** in commits or PRs (no Co-Authored-By lines).
- **Never guess** pinouts, ratings, footprints or dimensions; check the datasheet, the reference schematic or a measurement, or say it's unknown. Mark unchecked claims in the SPEC **(verify)**.
- **Search the whole repo** for related references when changing something.
- **Knowledge goes in the repo,** not the assistant's local memory: the owner works on several machines.
- Own design: draw from datasheets and the facts recorded in the SPEC. Seeed's XIAO symbol and footprint are used as published.
