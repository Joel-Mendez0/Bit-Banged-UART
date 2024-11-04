/*
 * UART.h
 *
 *  Created on: Oct 31, 2024
 *      Author: Joel
 */

#ifndef MAIN_UART_H_
#define MAIN_UART_H_

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_attr.h"
#include "esp_log.h"

#define LOW 0
#define HIGH 1

extern gptimer_handle_t clk_timer; 

typedef enum{
	PARITY_NONE = 0,
	PARITY_ODD = 1,
	PARITY_EVEN = 2
} ParityType;

typedef enum{
	BAUD_RATE_4800 = 208,
	BAUD_RATE_9600 = 104,
	BAUD_RATE_19200 = 52,
	BAUD_RATE_38400 = 26,
	BAUD_RATE_57600 = 17,
	BAUD_RATE_115200 = 9,
	BAUD_RATE_125000 = 8
} BAUD_RATE;

typedef struct {
    gpio_num_t RXPin;         // RX Pin for UART
    gpio_num_t TXPin;         // TX Pin for UART
    uint32_t BAUDRATE;        // Baud rate for communication
    ParityType parity;        // Parity Type
    uint8_t DataFrameSize;    // Data frame size in bits (5-8)
} UARTcfg;

/*UART PROTOTYPES*/
void InitUART(const UARTcfg *cfg);
void SendStartBit(void);
void SetDataFrame(uint8_t byte);
void SendBit(uint8_t bit);
void SendParityBit(void);
void SendStopBit(void);
void UARTTransmit(uint8_t *data, uint32_t length);
void UARTReceive(uint8_t *data, uint32_t length);

/*TIMER FUNCTIONS*/
void start_timer(uint64_t period);
void end_timer(void);

#endif /* MAIN_UART_H_ */
