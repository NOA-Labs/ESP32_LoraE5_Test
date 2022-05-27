#include <string.h>
#include "bsp_uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"

#define PATTERN_CHR_NUM     (3)

static const char *TAG = "uart_events";

void bsp_uart_config(uart_port_t num, int tx, int rx, int rx_buff_size, int tx_buff_size, void* queue)
{
    const uart_config_t uart_config = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    
    int intr_alloc_flags = 0;

    QueueHandle_t *q = (QueueHandle_t *)queue;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_param_config(num, &uart_config));
    //Install UART driver, and get the queue.
    ESP_ERROR_CHECK(uart_driver_install(num, rx_buff_size*2, tx_buff_size*2, 20, q, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_set_pin(num, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    //Set uart pattern detect function.
    ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(num, '+', PATTERN_CHR_NUM, 9, 0, 0));
    //Reset the pattern queue length to record at most 20 pattern positions.
    ESP_ERROR_CHECK(uart_pattern_queue_reset(num, 20));
}

void bsp_uart_send_data(uart_port_t num, const char*buff, size_t length)
{
    uart_write_bytes(num, buff, length);
}

int bsp_uart_receive_data(uart_port_t num, char *buff, void* queue, TickType_t ticks_to_wait)
{
    uart_event_t event;
    size_t buffered_size;
    QueueHandle_t *q = (QueueHandle_t *)queue;

    if(xQueueReceive(*q, (void * )&event, ticks_to_wait)){
        //  ESP_LOGI(TAG, "uart[%d] event:", num);
         switch(event.type) {
            case UART_DATA:
                return uart_read_bytes(num, buff, event.size, ticks_to_wait);
            case UART_FIFO_OVF: //Event of HW FIFO overflow detected
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(num);
                xQueueReset(*q);
            break;
             case UART_BUFFER_FULL://Event of UART ring buffer full
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(num);
                xQueueReset(*q);
                break;
            case UART_BREAK://Event of UART RX break detected
                ESP_LOGI(TAG, "uart rx break");
                break;
            case UART_PARITY_ERR://Event of UART parity check error
                ESP_LOGI(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR://Event of UART frame error
                ESP_LOGI(TAG, "uart frame error");
                break;
            case UART_PATTERN_DET://UART_PATTERN_DET
                uart_get_buffered_data_len(num, &buffered_size);
                int pos = uart_pattern_pop_pos(num);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(num);
                } else {
                    uart_read_bytes(num, buff, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(num, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", buff);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            //Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
         }
    }
    
    return 0;
}