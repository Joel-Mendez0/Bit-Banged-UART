
# Bit-Banged UART on ESP32

This project implements a UART (Universal Asynchronous Receiver-Transmitter) protocol using a bit-banging approach on the ESP32 with ESP-IDF. By directly controlling GPIO pins and utilizing hardware timers, it simulates UART communication, providing a flexible way to transmit and receive data without relying on the hardware UART peripheral.

## Features

- **Configurable Baud Rates**: Supports baud rates from 4800 up to 125000.
- **Parity Options**: Includes parity settings (None, Odd, Even) for error-checking.
- **Data Frame Size**: Allows configuration of data frames between 5-8 bits.
- **Start, Stop, and Parity Bits**: Implements full UART protocol with start, stop, and optional parity bits.
- **Hardware Timer Integration**: Uses ESP32’s GPTimer to accurately control bit timing.

## Project Structure

- **UART.h**: Header file defining the UART configuration structure, enums, and function prototypes.
- **UART.c**: Main UART implementation, including functions to initialize UART, send/receive data, and manage GPIO and timers.

## Code Overview

### Initialization

The `InitUART()` function sets up the RX and TX GPIO pins, configures baud rate, parity, and data frame size, and initializes a timer for precise bit timing.

```c
UARTcfg cfg = {
    .RXPin = 22,
    .TXPin = 23,
    .BAUDRATE = BAUD_RATE_125000,
    .DataFrameSize = 8,
    .parity = PARITY_ODD
};

InitUART(&cfg);
```

### Data Transmission

The `UARTTransmit()` function sends a byte array using bit-banging, adhering to the UART protocol. Start, parity, and stop bits are managed with accurate timing via the configured timer.

```c
uint32_t length = 10;
uint8_t data[length];
for(uint32_t i = 0; i < length; i++){
    data[i] = i;
}

UARTTransmit(data, length);
```

### Data Reception

`UARTReceive()` reads data from the RX pin by monitoring for the start condition, then clocks in each bit and verifies the parity (if configured). The received byte is stored in the provided data array.

```c
uint8_t received_data[10];
UARTReceive(received_data, 10);
```

## Baud Rate Configuration

This implementation supports multiple common UART baud rates through predefined values:

| Baud Rate | Alarm Count |
|-----------|-------------|
| 4800      | 208         |
| 9600      | 104         |
| 19200     | 52          |
| 38400     | 26          |
| 57600     | 17          |
| 115200    | 9           |
| 125000    | 8           |

These settings are based on a timer resolution of 1 MHz, providing precise timing for each bit transmission.

## How to Run

1. Clone this repository and set up your ESP32 development environment with ESP-IDF.
2. Modify the `RXPin` and `TXPin` in `app_main()` as needed.
3. Build and flash the code to your ESP32.
4. Connect the configured TX/RX pins to a UART receiver to observe transmission.

```sh
idf.py build
idf.py flash
```

## Applications

This bit-banging UART implementation can be used in scenarios where the hardware UART peripherals are occupied, or for educational purposes to understand the fundamentals of UART communication.

## License

This project is licensed under the MIT License.
