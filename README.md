# ESP8266 Multi-Device Synchronized Lights

Ultra-precise (<1ms) synchronized LED lighting system using ESP-NOW mesh networking on ESP8266 microcontrollers.

## Description

This project implements a wireless mesh network of ESP8266 devices that control LED strips with sub-millisecond synchronization. Using ESP-NOW protocol, multiple ESP8266 nodes coordinate to display perfectly synchronized lighting effects across all connected devices.

## Features

- **Sub-millisecond Synchronization** - Lights sync with <1ms precision
- **ESP-NOW Mesh** - Wireless coordination without WiFi router
- **Multi-Device Support** - Unlimited number of synchronized nodes
- **Low Latency** - Direct peer-to-peer communication
- **Reliable** - No internet connection required
- **Scalable** - Easy to add more light nodes

## System Architecture

Part of a complete lighting control system:

```
iOS App → ESP32 BLE Hub → ESP8266 Mesh Network
                              ↓ ESP-NOW ↓
                          [ESP8266] [ESP8266] [ESP8266]
                             (Synchronized <1ms)
```

## Related Projects

- **[Lights_ios](https://github.com/kochcodes/Lights_ios)** - iOS control app
- **[Lights_ESP32](https://github.com/kochcodes/Lights_ESP32)** - ESP32 BLE hub coordinator

## Hardware Requirements

- **ESP8266 Development Boards** (NodeMCU, Wemos D1 Mini, etc.)
- **LED strips** (WS2812B or similar addressable LEDs)
- **Power supply** (5V for LEDs and ESP8266)
- Optional: Level shifter for LED data signal

## Installation

### Prerequisites

- [PlatformIO](https://platformio.org/) or Arduino IDE
- USB cable for programming
- Multiple ESP8266 boards for mesh

### Build and Upload

1. Clone the repository:
```bash
git clone https://github.com/kochcodes/ESP8266MultiDeviceSyncLight.git
cd ESP8266MultiDeviceSyncLight
```

2. Build and upload to each ESP8266:
```bash
pio run -t upload
```

3. Repeat for all ESP8266 nodes in your mesh

## Configuration

### platformio.ini

Configure your ESP8266 board:
```ini
[env:d1_mini]
platform = espressif8266
board = d1_mini
framework = arduino
```

### Mesh Setup

1. Flash firmware to all ESP8266 devices
2. Power on all devices - they auto-discover via ESP-NOW
3. Mesh network forms automatically
4. Synchronization begins immediately

## How It Works

### ESP-NOW Protocol

- Uses ESP-NOW for direct device-to-device communication
- No WiFi router required
- Sub-millisecond latency
- Broadcasts commands to all mesh nodes simultaneously

### Synchronization

All devices receive commands at nearly the same instant, ensuring:
- Perfectly timed animations
- Coordinated color changes
- Synchronized effects across all lights

## Usage

### Standalone Mode

The system can run independently or be controlled by the ESP32 BLE hub which receives commands from the iOS app.

### Animation Modes

Supports various synchronized effects:
- Solid colors
- Fading
- Rainbow
- Custom animations

## Development

### Project Structure

```
ESP8266MultiDeviceSyncLight/
├── src/              # Source code
│   └── main.cpp      # Main application
├── platformio.ini    # PlatformIO configuration
└── Makefile          # Build automation
```

## Troubleshooting

### Devices Not Syncing
- Ensure all devices are on the same WiFi channel
- Check ESP-NOW initialization succeeded
- Verify MAC addresses are correctly configured

### Timing Issues
- Minimize code in interrupt handlers
- Use hardware timers where possible
- Reduce WiFi activity during critical timing

## Technical Details

- **Synchronization Accuracy**: <1ms between nodes
- **Maximum Nodes**: Limited by ESP-NOW (20 paired devices)
- **Range**: ~100m line of sight
- **Protocol**: ESP-NOW (802.11 based)

## License

Open source - check repository for license details.

## Tags

`esp8266` `esp-now` `mesh-network` `led` `lighting` `synchronized` `iot` `home-automation` `platformio` `wireless`
