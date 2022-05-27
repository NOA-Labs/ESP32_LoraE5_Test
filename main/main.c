#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "lora_task.h"

void app_main(void)
{
    printf("system start.\r\n");
    
    lora_init_task(10);

    while(1){
        vTaskDelay(10000);
    }
}
