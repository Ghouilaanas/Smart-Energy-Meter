# 📟⚡ Smart-Energy-Meter
This project implements a prototype Smart Energy Meter developed on an Arduino Uno board powered by the ATmega328P microcontroller, designed to provide real-time monitoring and control of household or industrial electrical loads. The prototype focuses on enhancing traditional metering systems by enabling remote data collection, energy consumption analysis, and load status supervision through IoT-based communication.
## 📄 Description
The system integrates the ACS712 current sensor to measure the current drawn by various electrical loads such as 220V lights, fans, and other appliances, ensuring accurate monitoring of individual and total power consumption. The Arduino Uno serves as the central processing unit, handling current measurement, signal processing, and energy usage calculation in real-time.

For communication, the prototype employs the SIM800L GSM/GPRS module, allowing the system to transmit consumption data to a remote user or server via SMS. The system further incorporates load status detection, making it possible to identify active appliances and assist in reducing overall energy consumption through informed decision-making.

## ⚙️ Tools and Technologies

#### 1. Hardware components
- **Microcontroller**: STM32F407 Discovery
- **Sensor**: Analog Infrared Sensor **Sharp**
- **Display**: I²C LCD 16x2
- **Programming**: STM32CubeIDE / HAL library (Hardware Abstraction Layer)

#### 1.2 Hardware connections
- **Sharp** : 
  - VCC → 5V
  - OUT → PA1 (ADC1_IN1 sur STM32)
  - GND → GND
- **LCD** :
  - VCC → 5V
  - SDA → PB7 (Bus I2C1 sur STM32)
  - SCL → PB6 (Bus I2C1 sur STM32)
  - GND → GND
---
## 📖 Guide to Use
1. Clone this repository:
git clone https://github.com/Ghouilaanas/Traffic-flow-detection-system.git
2. 📂 **Open the STM32CubeIDE project** corresponding to the desired traffic flow detection approach.
3. 🛠️ **Build and flash the firmware** for that approach onto the appropriate STM32 board.
4. 🔌 **Connect the sensors, microcontroller, and display** following the wiring diagram for the selected setup.
5. ⚡ **Power on the system** and **observe the real-time traffic flow readings** on the LCD.
