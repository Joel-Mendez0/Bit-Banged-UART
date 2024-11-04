/*
 * UART.c
 *
 *  Created on: Oct 31, 2024
 *      Author: Joel
 */

#include "UART.h"

static const char *TAG = "UART";

//esp_timer_handle_t clk_timer;
gptimer_handle_t clk_timer; 

// Static variables to hold the state of the protocol
static volatile uint8_t elapsed_cycles = 0;
static volatile uint8_t byte_to_send;
static volatile uint8_t parity_value = 0;
static volatile uint8_t timer_active = 0;
static volatile uint8_t received_byte = 0;

// Static variables to hold the UART configuration settings
static gpio_config_t RXconfig;
static gpio_config_t TXconfig;
static gpio_num_t RX;
static gpio_num_t TX;
static uint32_t BAUDRATE;
static ParityType parity;
static uint8_t DataFrameSize;

static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *event_data, void *param) {
    switch(elapsed_cycles) {
        case 0: 
            SendStartBit();
            break;
        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8:
            SendBit(byte_to_send & 0x01);
            parity_value ^= (byte_to_send & 0x01);
            byte_to_send >>= 1;
            break;
        case 9:
            if(parity != PARITY_NONE) {
                SendParityBit();
                break;
            }
            SendStopBit();
            parity_value = 0;
            elapsed_cycles = 0;
            timer_active = 0;
            break;
        case 10:
            SendStopBit();
            parity_value = 0;
            elapsed_cycles = 0;
            timer_active = 0;
            break;
        default:
            break;
    }
    
    elapsed_cycles++;
    return false;
}

void InitUART(const UARTcfg* cfg) {
    // Dereference cfg to access the configuration settings
    RX = cfg->RXPin;
    TX = cfg->TXPin;
    BAUDRATE = cfg->BAUDRATE;
    parity = cfg->parity;
    DataFrameSize = cfg->DataFrameSize;
    
    RXconfig = (gpio_config_t){
        .pin_bit_mask = (1ULL << RX),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    TXconfig = (gpio_config_t){
        .pin_bit_mask = (1ULL << TX),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    gpio_config(&RXconfig);
    gpio_config(&TXconfig);
    
    // TX IDLE STATE
    gpio_set_level(TX, HIGH);

	// Hardware Timer Setup
	gptimer_config_t timer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = 1000000,
	};
	gptimer_new_timer(&timer_config,&clk_timer);
	
	gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
	
	gptimer_alarm_config_t alarm_config = {
		.reload_count = 0,
		.alarm_count = BAUDRATE,
		.flags.auto_reload_on_alarm = true
	};
	
	gptimer_set_alarm_action(clk_timer, &alarm_config);
    gptimer_register_event_callbacks(clk_timer, &cbs, NULL);
	gptimer_enable(clk_timer);
	
	
	//Call Transmit once during initialization to stabilize the timing
	uint8_t stable[1] = {0};
	UARTTransmit(stable, 1);
}

void start_timer(uint64_t period) {
	
    gptimer_set_alarm_action(clk_timer, &(gptimer_alarm_config_t){
        .alarm_count = period,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true
    });
    
    gptimer_start(clk_timer);
}

void end_timer(void) {
    gptimer_stop(clk_timer);
}

void SendStartBit(void){
	
	/*UART pulls transmission line LOW for one clock cycle*/
	gpio_set_level(TX,LOW);
}
void SendStopBit(void){
	
	/*UART pulls transmission line HIGH*/
	gpio_set_level(TX,HIGH);
}
void SetDataFrame(uint8_t byte){
	byte_to_send = byte;
}
void SendBit(uint8_t bit){
	gpio_set_level(TX,bit);
}
void SendParityBit(void){
	/*parity_value is 0 when there are an even amount of 1s*/
	if(parity == PARITY_EVEN){
		SendBit(parity_value);
	}
	else if(parity == PARITY_ODD){
		SendBit(!parity_value);
	}
}
void UARTTransmit(uint8_t *data, uint32_t length) {
 	
    start_timer(BAUDRATE);
    for(uint32_t i = 0; i < length; i++) {
        timer_active = 1;
        elapsed_cycles = 0;
        SetDataFrame(data[i]);  
        while(timer_active);
    }
    end_timer();
}
void UARTReceive(uint8_t *data, uint32_t length){
	for(uint32_t i = 0; i < length; i++){
		/* 1 Detect Start Condition*/
		while (gpio_get_level(RX) == HIGH);
		
		/* 2 Start Timer such that we clock in data in the middle of a cycle*/
		start_timer(BAUDRATE/2);
		
		/* 3 Take the bit and shift it into a uint8_t data variable*/
		for(uint8_t bit = 0; bit < DataFrameSize; bit++){
			
			while(!elapsed_cycles);
			uint8_t current_bit = gpio_get_level(RX);
			received_byte |= (current_bit << bit);
			parity_value ^= current_bit;
			elapsed_cycles = 0;
		}
		
		/* 4 Check for parity bit if enabled*/
		if(parity != PARITY_NONE){
			while(!elapsed_cycles);
			uint8_t parity_bit = gpio_get_level(RX);
			if(parity == PARITY_EVEN && parity_bit != parity_value){
				ESP_LOGE(TAG, "Parity error (Even)");
			}
			else if(parity == PARITY_ODD && parity_bit == parity_value){
				ESP_LOGE(TAG, "Parity error (Odd)");
			}
			elapsed_cycles = 0;
		}
		
		/* 5 Check for Stop Condition*/
		while(!elapsed_cycles);
		
		
		/* 6 Take the byte and place it in the data array*/
		data[i] = received_byte;
		
	
		elapsed_cycles = 0;
	}
	end_timer();
	
	
}