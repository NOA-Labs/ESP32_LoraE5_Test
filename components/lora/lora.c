#include "lora.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include "esp_log.h"
#include "uart_tokenizer.h"

static const char*TAG = "lora";
/* tokenizer config */
static uint8_t rn2483_setup_lineBuffer[200] = {0};
static stream_char_tokenizer_result_t rn2483_setup_resultBuffer = {0};
static stream_char_tokenizer_config_t rn2483_setup_tokenizerConfig = {
   .token = '\n',
   .bufferMaxLen = 190,
   .data = rn2483_setup_lineBuffer,
   .result = &rn2483_setup_resultBuffer
};

/* variables used for response checking */
char* setupCmdExpectedResponse;
uint8_t setupCmdGotCorrectResponse;

/* function used to reinitialize uart with different baudrate */
void prvLoraChangeBaudRate( uint32_t newBaudRate){
}

/* response parser function */
void rn2483_setup_cmd_process_data_impl(const uint8_t* data, size_t len){
    const uint8_t *d2 = data;
    while (len--){

		/* 
         the following could be useful to debug command response from the lora module.
         it just forwards all received characters to our debug uart:

         while (!LL_USART_IsActiveFlag_TC(UART5)) {}		
         LL_USART_TransmitData8(UART5, *d2);
      */

		/* tokenize by line */
		stream_char_tokenizer_result_t* result = stream_tokenize_by_char(&rn2483_setup_tokenizerConfig, *d2++);
		if(result == NULL){ continue; }

		/* check response */
		if(setupCmdExpectedResponse && strncmp((char*)result->data, setupCmdExpectedResponse, strlen(setupCmdExpectedResponse)) == 0){
			setupCmdGotCorrectResponse = 1;
		}
	 }
}

uint8_t _lora_setup_cmd(char* cmd, char* expectedResponse){
	
	/* setup response check */
	setupCmdExpectedResponse = expectedResponse;
	setupCmdGotCorrectResponse = 0;
	
	/* send command */
	bsp_e5_send_cmd(cmd);

    char rx_buff[400] = {0};
    int ret = bsp_e5_recieve_data(rx_buff, 2000);

    if(ret > 0){
        rn2483_setup_cmd_process_data_impl((const uint8_t *)rx_buff, strlen(rx_buff));
    }
    else{
        printf("\r\nLORA Setup CMD Rx timed out.");
        return 0;
    }

    printf("response : %s\r\n", rx_buff);

    if(setupCmdGotCorrectResponse){
        return 1;
    }
    
    return 0;
	// uint32_t startTime = xTaskGetTickCount();

	// /* receive data 1 sec timout */
	// while(1){
	// 	if( xSemaphoreTake( rn2483_sentence_parser_task_get_semaphore(), 1000 ) == pdTRUE ){
	// 		dma_lora_uart_rx_check(&rn2483_setup_cmd_process_data_impl);
	// 		if(setupCmdGotCorrectResponse){
	// 			return 1;
	// 		}
		
	// 		if (xTaskGetTickCount() - startTime > 2000) {
	// 			return 0;
	// 		}
	// 	}else{
	// 		printf("\r\nLORA Setup CMD Rx timed out.");
	// 		return 0;
	// 	}
	// }
}

uint32_t _lora_get_own_device_id() {
    return 0;
	// return *(uint32_t*)0x1FFF7590;
}

void _lora_initialize(void){

   /* give the module some time to power up */
	vTaskDelay(200);
	printf("\r\nTrying LORA com on %d baud...", 115200);

	/* check communication at given baudrate */
	if(_lora_setup_cmd("AT\r\n", "+AT: OK")){
		printf("\r\nLORA changing baud rate to 115200...");
		vTaskDelay(100);

		/* seems we're running on 9600 baud. switch to 115200 */
		if(_lora_setup_cmd("AT+UART=BR,115200\r\n", "+UART: BR, 115200")){
			printf("\r\nLORA reported Switch Baud okay. Now resetting.");
			vTaskDelay(100);

			if(_lora_setup_cmd("AT+RESET\r\n", "+RESET: OK")){
				printf("\r\nLORA Reset Done.");

				/* switch baud rate */
				prvLoraChangeBaudRate(115200);
				printf("\r\nLORA USART: Changed baudrate 115200");				
			}
		}
	}else{

		/* seems that the module is already configured to 115200 baud. change baud rate */
		prvLoraChangeBaudRate(115200);
		printf("\r\nLORA USART: Seems running on 115200. Changed baudrate 115200");
	}

   /* check com max 3. times */
	uint32_t loraInitialized = 0;
	for(uint32_t i = 0; i < 3; i++){
		vTaskDelay(100);
		if(_lora_setup_cmd("AT\r\n", "+AT: OK")){
			loraInitialized = 1;
			break;
		}
	}

   /* at this point we should have a working lora configuration. otherwise disbale lora */
   assert_lora_cfg(loraInitialized);
	printf("\r\nLORA Init okay.");
	vTaskDelay(100);
}

void assert_failed_lora(uint8_t* file, uint32_t line){

   printf("\r\nLORA Assert failed. Disabling LORA.");
   while(1){
      vTaskDelay(10000);
   }
}

/* possible needed config */
// /* lbt threshold ignore */
// if(ulRN2483WaitResponse(e22UART.USARTx,xRn2483UartBinarySemaphore, "AT+LW=THLD,-1\r\n", "+LW: THLD, -1", 1000)){
//    printf("\r\nSet lbt threshold.");
// }else{
//    printf("\r\nFailed to set lbt threshold");
// }

// /* duty cycle check off */
// if(ulRN2483WaitResponse(e22UART.USARTx,xRn2483UartBinarySemaphore, "AT+LW=DC,OFF\r\n", "+LW: DC, OFF, 0", 1000)){
//    printf("\r\nSet dc off.");
// }else{
//    printf("\r\nFailed to set dc off");
// }

// /* enable low data rate optimiziation */
// if(ulRN2483WaitResponse(e22UART.USARTx,xRn2483UartBinarySemaphore, "AT+LW=LDRO,ON\r\n", "+LW: LDRO, ON", 1000)){
//    printf("\r\nSet low data rate optimization.");
// }else{
//    printf("\r\nFailed to set low data rate optimization");
// } 

//bsp_e5_send_cmd("AT+LW=DC\r\n");
//bsp_e5_send_cmd("AT+LW=CDR\r\n");
//bsp_e5_send_cmd("AT+DR\r\n");
//bsp_e5_send_cmd("AT+DR=dr15\r\n");
//bsp_e5_send_cmd("AT+LW=DC,OFF\r\n");
//bsp_e5_send_cmd("AT+LW=THLD,-1\r\n"); /* default -85 */
