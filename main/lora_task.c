#include "lora_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lora.h"
#include "uart_tokenizer.h"
#include <string.h>

/* Pinout:
	G6 PA2 USART2_TX AF7
	H7 PA3 USART2_RX AF7
*/


SemaphoreHandle_t xRn2483UartBinarySemaphore = NULL;

/* tokenizer config */
static uint8_t rn2483_lineBuffer[200] = {0};
static stream_char_tokenizer_result_t rn2483_resultBuffer = {0};
static stream_char_tokenizer_config_t rn2483_tokenizerConfig = {
   .token = '\n',
   .bufferMaxLen = 190,
   .data = rn2483_lineBuffer,
   .result = &rn2483_resultBuffer
};

typedef struct {
  uint8_t identifier; /* always A */

  uint8_t gpsStatus;
  uint16_t gpsParsedAtTimeStamp;
  uint32_t latitude;
  uint32_t longitude;

  uint8_t gpsCogStatus;
  uint16_t lastValidGpsCog;
  int16_t correctedCog;
  uint16_t gprmcSpeed;
  uint32_t routeApStatus;
} _lora_rx_broadcast_response_t;

static const uint8_t nibble[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static char* privUint8ToHex(uint8_t num){
    static char ret[3] = {'0', '0', 0x00};
    ret[0] = nibble[(num & 0xf0) >> 4];
    ret[1] = nibble[(num & 0x0f)];
    return ret;
}

uint32_t _lora_send_packet(uint8_t* data, uint32_t dataLen) {

    bsp_e5_send_cmd("AT+TEST=TXLRPKT, \"");
    printf("AT+TEST=TXLRPKT, \"");

    for (uint32_t i = 0; i < dataLen; i++) {
        printf("%s ", privUint8ToHex(data[i]));
        bsp_e5_send_cmd(privUint8ToHex(data[i]));
    }

    printf("\"\r\n");
    bsp_e5_send_cmd("\"\r\n");

    return 1;
}

void rn2483_process_data_impl(const uint8_t* data, size_t len){
    const uint8_t *d2 = data;

    while (len--){

		/* tokenize by line */
		stream_char_tokenizer_result_t* result = stream_tokenize_by_char(&rn2483_tokenizerConfig, *d2++);
		if(result == NULL){ continue; }

		// if(strncmp((char*)result->data, "+AT: OK", strlen("+AT: OK")) == 0){
		// 	printf("\r\nLORA E5 Reported: AT okay");
		// }

		if(strncmp((char*)result->data, "+TEST: RX ", strlen("+TEST: RX ")) == 0){
			/* parse packet */
			e5_response_t* parsed = e5_packet_parse_response(result->data, result->bytesRead);
			if(parsed){
				// printf("\r\nLORA E5 Reported: %s Len: %d - Rec: %02X", result->data, parsed->data_len, parsed->data[0]);
				if (parsed->data[0] == 0xFF) {					
					/* remote command */
					if(parsed->data_len == 17 && *(uint32_t*)&parsed->data[1] == 0xABCDEF){

						printf("\r\nLORA Remote Command!");

						_lora_rx_remote_cmd_t* remoteCommand = (_lora_rx_remote_cmd_t*)&parsed->data[5];
						_lora_rx_remotecmd_response_t commandResponse = {
							.identifier = 0xCCCCCCCC,
							.packIdent = remoteCommand->packIdent,
							.address = remoteCommand->address,
							.payload = remoteCommand->payload
						};

						// commandResponse.payload = remotecommand_handle(remoteCommand);

						_lora_send_packet((uint8_t*)&commandResponse, sizeof(_lora_rx_remotecmd_response_t));
					} else if(parsed->data_len == 13 && *(uint32_t*)&parsed->data[1] == 0xDEADBEEF){
						// baitboatstate_t* state = baitboatstate_get_app_state();

						_lora_rx_broadcast_response_t commandResponse = {
							.identifier = 'A',
							.gpsStatus = 0,
							.gpsParsedAtTimeStamp = 0,
							.latitude = 0x20000,
							.longitude = 0x10000,
							.gpsCogStatus = 0,
							.lastValidGpsCog = 0,
							.correctedCog = 0,
							.gprmcSpeed = 0xffff,
							.routeApStatus = 0
						};

						_lora_send_packet((uint8_t*)&commandResponse, sizeof(_lora_rx_broadcast_response_t));
					} else if (parsed->data_len == (sizeof(lora_icq_t) + 1) && *(uint32_t*)&parsed->data[1] == LORA_ICQ_REQUEST_IDENTIFIER) {
						printf("\r\nReceived LORA icq response (ACK=yes)");
						lora_icq_t* icqPacket = (lora_icq_t*)&parsed->data[1];

                        printf(
                                "identifier         = 0x%x\n"
                                "boxDeviceId        = 0x%x\n"
                                "boatDeviceId       = 0x%x\n"
                                "proposedChannelId  = 0x%x\n"
                                "changeFrequency    = 0x%x\n"
                                "acknowledged       = 0x%x\n",
                                 icqPacket->identifier,
                                 icqPacket->boxDeviceId,
                                 icqPacket->boatDeviceId,
                                 icqPacket->proposedChannelId,
                                 icqPacket->changeFrequency,
                                 icqPacket->acknowledged);

						if (icqPacket->boatDeviceId == 0xFFFFFFFF) {
							_lora_send_icq(icqPacket->proposedChannelId, 0, 1, icqPacket->boxDeviceId);
							printf("\r\nSent LORA icq response");
						}
					}
				} else {
					/* no answer needed */
					if(parsed->data_len == 5){
						uint16_t val1 = *(uint16_t*)&parsed->data[1];
						uint16_t val2 = *(uint16_t*)&parsed->data[3];

						// if (!baitboatstate_get_pwmforward_state() && !baitboatstate_get_autodrive_armed()) {
						// 	_set_engine_throttle(val1, val2);
						// }
					} else if (parsed->data_len == (sizeof(lora_icq_t) + 1) && *(uint32_t*)&parsed->data[1] == LORA_ICQ_REQUEST_IDENTIFIER) {
						printf("\r\nReceived LORA icq response (ACK=no)");
						lora_icq_t* icqPacket = (lora_icq_t*)&parsed->data[1];

						if (icqPacket->boatDeviceId == _lora_get_own_device_id() && icqPacket->acknowledged && icqPacket->changeFrequency) {
							_lora_switch_channel(icqPacket->proposedChannelId);
						}
					}
				}
			}

			// baitboatstate_set_signallost_state(0);
		}

		if(strncmp((char*)result->data, "+TEST: TX DONE", strlen("+TEST: TX DONE")) == 0){
			bsp_e5_send_cmd("AT+TEST=RXLRPKT\r\n");
			// printf("\r\nLORA E5 Reported: TX done");
		}
	}
}

static void prvLoraTask( void *pvParameters )
{
    char rx_buff[400] = {0};

	printf("\r\nDevice id is %d", _lora_get_own_device_id());

	/* setup io and uart */
	printf("\r\nSetting up E5-USART");
	bsp_e5_uart_config();

	/* init lora */
	_lora_initialize();

	/* setup test mode */
	assert_lora_cfg(_lora_setup_cmd("AT+MODE=TEST\r\n", "+MODE: TEST"));
	vTaskDelay(100);

	/* disable ldro */
	assert_lora_cfg(_lora_setup_cmd("AT+LW=LDRO,OFF\r\n", "+LW: LDRO, OFF"));
	vTaskDelay(100);

	/* setup radio cfg */
	assert_lora_cfg(_lora_setup_cmd("AT+TEST=RFCFG,868,SF7,500,12,15,22,ON,OFF,ON\r\n", "+TEST: RFCFG"));
	vTaskDelay(100);
	
	/* lora config done */
	printf("\r\nRadio configuration done.");
	
	/* Kick off receiving of packages */
	bsp_e5_send_cmd("AT+TEST=RXLRPKT\r\n");

	/* start com loop */
	while(1){
        if(bsp_e5_recieve_data(rx_buff, 5000) > 0){
            rn2483_process_data_impl((const uint8_t *)rx_buff, strlen(rx_buff));
            memset(rx_buff, 0, sizeof(rx_buff));
        }
        else{
            // We have not received any packages for 5sec - revert to default channel
			_lora_switch_channel(LORA_DEFAULT_COM_CHANNEL);
			// baitboatstate_set_signallost_state(1);

			/* Start receiving again to be extra sure */
			bsp_e5_send_cmd("AT+TEST=RXLRPKT\r\n");
			printf("\r\nLORA Rx timed out.");
        }
	}
}

void lora_init_task(UBaseType_t uxPriority){
	// xRn2483UartBinarySemaphore = xSemaphoreCreateBinary();
    xTaskCreate(prvLoraTask,
              "lora task",
              4096,
              NULL,
              uxPriority,
              NULL
              );
}

SemaphoreHandle_t rn2483_sentence_parser_task_get_semaphore(){
    return xRn2483UartBinarySemaphore;
}
