# Simple 7-Segment Display Control with ESP32 (FireBeetle v1.0)

This project demonstrates how to control a 7-segment display using an ESP32 FireBeetle board. It includes a web server setup on the ESP32 to update the display via a web interface.

## Features

- Control a 7-segment display using ESP32.
- Web interface for updating display content.
- Easy setup and use.

## Getting Started

### Prerequisites

- ESP32 FireBeetle board v1.0
- Adafruit 7-segment display with I2C backpack
- Breadboard and jumper wires

### Hardware Setup

1. Connect the 7-segment display to the ESP32 according to the provided wiring diagram.
2. Ensure a stable power supply for the ESP32 board.

### Software Installation

1. Clone the repository to your local machine.

   ```bash
   git clone https://github.com/antoinebou12/ESP32-7SEG.git
   ```
   
2. Open the project with your preferred IDE or editor that supports PlatformIO or the Arduino IDE.
3. Install the necessary libraries mentioned in the `platformio.ini` file or the Arduino project.
4. Upload the code to your ESP32 FireBeetle board.

## Usage

After uploading the code to your ESP32 board:

1. Connect the ESP32 to a power source.
2. The ESP32 starts a web server accessible within your local network.
3. Find the ESP32's IP address from your serial monitor and visit the provided IP using a web browser.
4. Use the web interface to update the 7-segment display's content.

## Contributing

Contributions to improve the project are welcome. Please feel free to fork the repository and submit pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
