#include "bsp_e5_drv.h"
#include <string.h>

#define TXD_PIN             (25)
#define RXD_PIN             (33)
#define EX_UART_NUM         UART_NUM_1
#define RX_BUFF_SIZE        (400)
#define TX_BUFF_SIZE        (128)

static QueueHandle_t uart0_queue;

void bsp_e5_uart_config(void)
{
    bsp_uart_config(EX_UART_NUM, TXD_PIN, RXD_PIN, RX_BUFF_SIZE, 0, &uart0_queue);
}

void bsp_e5_send_cmd(const char* cmd)
{
    bsp_uart_send_data(EX_UART_NUM, cmd, strlen(cmd));
}

int bsp_e5_recieve_data(char *rx_buff, TickType_t ticks_to_wait)
{
    return bsp_uart_receive_data(EX_UART_NUM, rx_buff, &uart0_queue, ticks_to_wait);
}

