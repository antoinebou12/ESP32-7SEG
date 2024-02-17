# ESP32 7-Segment Display Controller

![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg) ![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)

This repository houses a project that simplifies controlling a 7-segment display using an ESP32 FireBeetle board. The key feature of this project is its web-based interface, allowing users to update the display content over a local network.

## Features

- **Direct Control Over 7-Segment Display**: Use the ESP32 FireBeetle board to control what is displayed, from digits to specific characters.
- **Web Interface for Real-time Updates**: A built-in web server on the ESP32 lets you change the display content through any browser on your local network.
- **Comprehensive Documentation**: Detailed setup and usage instructions ensure a smooth start, regardless of your experience level with hardware projects.

## Getting Started

### Prerequisites

- **Hardware**: ESP32 FireBeetle board (v1.0), Adafruit 7-segment display (I2C backpack), breadboard, and jumper wires.
- **Software**: Git, PlatformIO or Arduino IDE, and necessary ESP32 libraries.

### Hardware Setup

1. **Display Connections**: Connect the Adafruit 7-segment display to the ESP32 using the I2C interface. Make sure to connect the SCL and SDA pins correctly, along with VCC and GND for power.
2. **Power Supply**: Power the ESP32 board through USB or an external power supply to ensure stable operation.

### Software Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/antoinebou12/ESP32-7SEG.git
   ```
2. **Open and Configure the Project**: Load the project into PlatformIO or the Arduino IDE. Install any required libraries as specified in `platformio.ini` or the project documentation.
3. **Upload to the ESP32**: Compile and upload the code to your ESP32 board to start the project.

## Usage

- **Access the Web Server**: After booting, the ESP32 launches a web server. Find its IP address in the serial monitor and navigate to it using a web browser.
- **Update Display via Web Interface**: Use the web interface to input and submit the content you want to be displayed. The ESP32 will update the 7-segment display in real-time.

## Contributing

We welcome contributions! If you have suggestions for improvements or find a bug, please feel free to fork the repository, make your changes, and submit a pull request.

## License

This project is available under the MIT License. For more details, see the [LICENSE](LICENSE) file.

## Acknowledgments

- **ESP32 FireBeetle Board**: A compact and versatile development board perfect for IoT projects.
- **Adafruit**: For their high-quality 7-segment display and excellent documentation.

---

Crafted with ❤️ by [Antoine Boucher](https://github.com/antoinebou12). For questions or feedback, please open an issue in the repository.
