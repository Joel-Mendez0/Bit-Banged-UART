#include <stdio.h>
#include <unistd.h>
#include "UART.h"

void app_main(void)
{
	
	UARTcfg cfg = {
		.RXPin = 22,
		.TXPin = 23,
		.BAUDRATE = BAUD_RATE_125000,
		.DataFrameSize = 8,
		.parity = PARITY_ODD
	};
	
	InitUART(&cfg);
	
	uint32_t length = 10;
	uint8_t data[length];
	
	for(uint32_t i = 0; i < length; i++){
		data[i] = i;
	}
	
	sleep(3); // Simulate Break Condition
	UARTTransmit(data, length);
	
}
