/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_H
#define __LORA_H

#include "freertos/FreeRTOS.h"
#include "bsp_e5_drv.h"

#define LORA_DEFAULT_COM_CHANNEL 7
#define LORA_ICQ_REQUEST_IDENTIFIER 0xAAAAAA
#define LORA_ICQ_RESPONSE_IDENTIFIER 0xBBBBBB

#define xUartDebug_printf       printf

typedef struct {
    uint32_t data_len;
    uint8_t data[200];
} e5_response_t;

typedef struct {
    uint32_t identifier;
    uint32_t boxDeviceId;
    uint32_t boatDeviceId;
    uint32_t proposedChannelId;
    uint32_t changeFrequency;
    uint32_t acknowledged;
} lora_icq_t;

#define assert_lora_cfg(expr) ((expr) ? (void)0 : assert_failed_lora((uint8_t *)__FILE__, __LINE__))
void assert_failed_lora(uint8_t* file, uint32_t line);

void _lora_initialize(void);
uint8_t _lora_setup_cmd(char* cmd, char* expectedResponse);
uint32_t _lora_send_packet(uint8_t* data, uint32_t dataLen);
uint32_t _lora_get_own_device_id();
e5_response_t* e5_packet_parse_response(uint8_t* rx, uint32_t len);

void _lora_send_icq(uint32_t proposedChannel, uint32_t changeFrequency, uint32_t acknowledged, uint32_t targetDeviceId);
void _lora_switch_channel(uint32_t suggestedChannel);

#endif
