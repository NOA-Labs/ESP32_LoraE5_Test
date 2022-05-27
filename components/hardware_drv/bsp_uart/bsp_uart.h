#pragma once

#include "driver/uart.h"

#ifdef __cplusplus
    extern "C" {
#endif

void bsp_uart_config(uart_port_t num, int tx, int rx, int rx_buff_size, int tx_buff_size, void* queue);
void bsp_uart_send_data(uart_port_t num, const char*buff, size_t length);
/**
 * @brief 
 * 
 * @param num 
 * @param buff 
 * @param queue 
 * @param ticks_to_wait 
 * @return int -1: error .
 */
int bsp_uart_receive_data(uart_port_t num, char *buff, void* queue, TickType_t ticks_to_wait);

#ifdef __cplusplus
    }
#endif