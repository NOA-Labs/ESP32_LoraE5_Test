/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_TASk_H
#define __LORA_TASk_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct{
    uint16_t packIdent;     /* Random Pack Ident */
    uint16_t address;       /* 0-15 Address, Bit 16 R/W */
    uint32_t targetNode;
    uint32_t payload;
} _lora_rx_remote_cmd_t;

typedef struct{
	uint32_t identifier;	/* always 0xCCCCCCCC */
    uint16_t packIdent;     /* Random Pack Ident */
    uint16_t address;       /* 0-15 Address, Bit 16 R/W */
    uint32_t payload;
} _lora_rx_remotecmd_response_t;

void lora_init_task(UBaseType_t uxPriority);
SemaphoreHandle_t rn2483_sentence_parser_task_get_semaphore();

#endif
