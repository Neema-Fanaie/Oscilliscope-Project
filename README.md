# Oscilloscope Project

A DIY digital oscilloscope built on an Arduino Uno, sampling an analog signal and rendering the waveform live on an OLED display.

## Overview

[Project Description]

<!-- [1–2 sentences describing what the project does — e.g. "This project turns an Arduino Uno into a basic single-channel oscilloscope, sampling an analog input and drawing the waveform in real time on a small OLED screen."] -->

## Features

- Real-time waveform display on an OLED screen (via [u8g2](https://github.com/olikraus/u8g2))
- Circular/sample buffering using the [Queue](https://github.com/Fapitxu/Queue) library
<!-- - [Add: adjustable sample rate / voltage scaling / trigger mode / etc., if implemented]
- [Add: any input protection / attenuation circuitry, if used] -->

## Hardware Requirements

| Component | Notes |
|---|---|
| Arduino Uno | Or compatible ATmega328P board |
| OLED display | SSD1306/SH1106-class display compatible with u8g2 |
| [Probe / input circuitry] | [e.g. voltage divider, op-amp buffer, protection diodes] |
| Breadboard + jumper wires | |
| [USB cable] | For programming and serial power |

### Wiring

| Arduino Pin | Connects To | Purpose |
|---|---|---|
| [A2 / analog pin] | Signal probe input | Analog signal to be sampled |
| [SDA/SCL or SPI pins] | OLED display | Display communication |
| 5V / GND | OLED display, input circuit | Power |

[Add a wiring diagram or photo here if available.]

## Software Requirements

This is a [PlatformIO](https://platformio.org/) project targeting the `uno` environment.

- [PlatformIO Core](https://platformio.org/install/cli) or the [PlatformIO IDE extension for VS Code](https://platformio.org/install/ide?install=vscode)
- Arduino framework (installed automatically by PlatformIO)

### Dependencies

Declared in `platformio.ini` and installed automatically by PlatformIO:

- [`smfsw/Queue`](https://github.com/Fapitxu/Queue) — sample buffering
- [`olikraus/u8g2`](https://github.com/olikraus/u8g2) — OLED display driver/graphics library

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/Neema-Fanaie/Oscilliscope-Project.git
   cd Oscilliscope-Project
   ```
2. Open the folder in VS Code with the PlatformIO extension installed (or use the PlatformIO CLI).
3. Connect your Arduino Uno via USB.
4. Build and upload:
   ```bash
   pio run --target upload
   ```
5. Open the serial monitor if needed for debugging:
   ```bash
   pio device monitor
   ```

## Project Structure

```
.
├── include/                # Header files
├── lib/                    # Project-specific libraries
├── src/                    # Main application source (main.cpp)
├── Oscilliscope Project/   # [Describe contents — schematics, notes, extra files?]
├── platformio.ini          # PlatformIO project configuration
└── README.md
```

## How It Works

<!-- [Briefly describe the sampling/display pipeline, e.g.:] -->
1. The analog input pin is sampled at a fixed interval using `analogRead()` (or direct ADC register access).
2. Samples are pushed into a queue/ring buffer to hold a window of the waveform.
3. The buffered samples are scaled and drawn as a line/pixel plot on the OLED via u8g2.
4. The display refreshes continuously to show a "live" trace.

## Usage

<!-- [Describe how to interpret the display, any buttons/controls for scale or trigger, expected input voltage range, etc.] -->

## Limitations

- [e.g. Sampling rate limited by Arduino Uno's ADC speed — not suitable for high-frequency signals]
- [e.g. Input voltage range limited to 0–5V; external circuitry required for AC or higher-voltage signals]

## Roadmap / Possible Improvements

- [ ] [e.g. Add trigger functionality]
- [ ] [e.g. Add adjustable time/voltage scale]
- [ ] [e.g. Support multiple channels]

## Author

[Neema Fanaie](https://github.com/Neema-Fanaie)

## License

<!-- [Add a license, e.g. MIT — currently unspecified] -->
