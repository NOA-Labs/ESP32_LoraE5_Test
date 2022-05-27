#include "lora.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t currentChannel = LORA_DEFAULT_COM_CHANNEL;

static char* setChannelCommands[8]= {
	"AT+TEST=RFCFG,870\r\n",
	"AT+TEST=RFCFG,869\r\n",
	"AT+TEST=RFCFG,863\r\n",
	"AT+TEST=RFCFG,864\r\n",
	"AT+TEST=RFCFG,865\r\n",
	"AT+TEST=RFCFG,866\r\n",
	"AT+TEST=RFCFG,867\r\n",
	"AT+TEST=RFCFG,868\r\n",
};

void _lora_send_icq(uint32_t proposedChannel, uint32_t changeFrequency, uint32_t acknowledged, uint32_t targetDeviceId) {
	lora_icq_t icqPack = {0};

	icqPack.identifier = LORA_ICQ_RESPONSE_IDENTIFIER;
    icqPack.boxDeviceId = targetDeviceId;
    icqPack.boatDeviceId = _lora_get_own_device_id();
    icqPack.proposedChannelId = proposedChannel;
    icqPack.changeFrequency = changeFrequency;
    icqPack.acknowledged = acknowledged;

	_lora_send_packet((uint8_t*) &icqPack, sizeof(lora_icq_t));
}

void _lora_switch_channel(uint32_t suggestedChannel) {
	if (suggestedChannel != currentChannel) {		
		vTaskDelay(300);

		if (_lora_setup_cmd(setChannelCommands[suggestedChannel], "+TEST: RFCFG")) {
			currentChannel = suggestedChannel;
			xUartDebug_printf("\r\nLORA switched channel to %d", suggestedChannel);
		}
    }
}
