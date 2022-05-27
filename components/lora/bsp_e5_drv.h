#pragma once

#include "bsp_uart.h"

void bsp_e5_uart_config(void);
void bsp_e5_send_cmd(const char* cmd);
int bsp_e5_recieve_data(char *rx_buff, TickType_t ticks_to_wait);