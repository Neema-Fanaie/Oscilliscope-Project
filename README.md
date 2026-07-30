# Oscilloscope Project

This is a basic Arduino Oscilliscope Project utilisng the Arduino Uno R3, an SD1306/SH1106-class display, and MCP 6272 OpAmp. This project is primarily for fun, so the circuit design is a compromise between doing things "cheaply," and in a cool way to get pratice (reason for using an OpAmp).

## Features

- Real-time Waveform Display on an OLED Screen (via [u8g2](https://github.com/olikraus/u8g2))
- Ideal diode circuits to differentiate between a positive and negative signal on the regular arduino ADC
- Buttons to toggle screen settings and potentiometer to move the x-axis line

## Hardware Requirements

1. Sensing:
  - OpAmp: MCP 6272, Contains 2 OpAmps which can accuratly rectify signal without error
  - 4x 1N4148 diodes, 4x 10k resistors, 2x 1k resistors, 1x 10nF capacitor near the OpAmp Supply and 1x10μF capacitor near Power Supply Source (Manufactorer's recommended values for MCP6272)

2. External Components:
  - Arduino Uno or compatible ATmega328P board and USB cable for Programming
  - 4x Push buttons for User Input (optional, used in scaling)
  - 4x 10k resistors for Pull-down on Push Buttons
  - Potentiometer for moving reference point on screen (optional)
  - OLED display | SSD1306/SH1106-class display compatible with u8g2 |
  - Breadboard and Jumper Wires for Prototyping

3. Power Supply:
  - 9V source for OpAmp and Arduino
  - LM2931az voltage regulator to provide 5V for OLED, Buttons and Potentiometer

### Wiring

[TO BE ADDED SOON]

## Software Requirements

This is a [PlatformIO](https://platformio.org/) project targeting the `uno` environment.

- [PlatformIO Core](https://platformio.org/install/cli) or the [PlatformIO IDE extension for VS Code](https://platformio.org/install/ide?install=vscode)
- Arduino Framework (installed automatically by PlatformIO)
- U8g2 Library

### Dependencies

Declared in `platformio.ini` and installed automatically by PlatformIO:

- [`olikraus/u8g2`](https://github.com/olikraus/u8g2) — OLED display driver/graphics library

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/Neema-Fanaie/Oscilliscope-Project.git
   cd Oscilliscope-Project
   ```
2. Open the folder in VS Code with the PlatformIO extension installed (or use the PlatformIO CLI).
3. Connect your Arduino Uno via USB.
4. Setup Circuit as Shown in the Wiring Diagram
5. Build and upload:
   ```bash
   pio run --target upload
   ```
6. Open the serial monitor if needed for debugging:
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

1. The analog input pin is sampled at a fixed interval with direct ADC register access.
2. Samples are pushed into a ring buffer to hold a window of the waveform.
3. The buffered samples are scaled and drawn as a line/pixel plot on the OLED via u8g2.
4. The display refreshes continuously to show a "live" trace.

## Usage

[TO BE ADDED SOON]

## Limitations

- The Arduino Uno's ADC is limited to 10-bit resolution and a maximum sampling rate of ~10 kHz (practical limit due to processing overhead).
- Input voltage should be within 0–5V unless external circuitry is used for scaling.
- OpAmp begins to fail at 15kHz on the rising edge of the signal, but can be usale up to 20kHz. Maybe experiment with different opamps and mcs to get better results.

## Roadmap / Possible Improvements

- [ ] Add Trigger Functionality
- [ ] Support multiple channels

## Author

[Neema Fanaie](https://github.com/Neema-Fanaie)