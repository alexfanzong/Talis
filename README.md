# Talis

Talis is an offline travel-assistance firmware prototype for the
SparkleIoT T5AI development board. The device presents a local action pack,
supports touch and button navigation, advertises a BLE service, and can drive
on-device haptic feedback without depending on a network connection.

## Repository scope

This repository contains the embedded firmware source only. Delivery notes,
firmware binaries, IDE metadata, agent configuration, the unused mini-app
scaffold, and private audio assets are intentionally excluded.

The checked-in audio data file is an empty placeholder. To enable spoken help,
replace it with an appropriately licensed 16 kHz mono MP3 byte array before
building.

## Hardware and SDK

- Target: T5AI
- Board: `SPARKLEIOT_T5AI_DEV`
- SDK: [TuyaOpen](https://github.com/tuya/TuyaOpen)
- Firmware entry point: `source/embedded/src/tuya_app_main.c`

## Build

Install the TuyaOpen SDK, then run from `source/embedded`:

```bash
tos.py check
tos.py build
```

Build products are written under `.build/` and `dist/`; they are ignored by
Git and are not part of this source repository.

## Current prototype boundaries

- The embedded travel content is demonstration data and must be reviewed
  before real-world use.
- The repository does not include a phone app.
- BLE, haptic, audio, and navigation behavior require verification on the
  target board after any source change.
- The private voice recording used in the original delivery package is not
  included.

## License and attribution

See [LICENSE](LICENSE). The local board-registration implementation is adapted
from the TuyaOpen SDK and retains attribution in its source header.
