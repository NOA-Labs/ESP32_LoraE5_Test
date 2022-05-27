#include "lora.h"
#include <string.h>

uint8_t privHexToString(uint8_t char0, uint8_t char1) {
uint8_t out = 0;
	uint8_t current = char0;

	// 0-9
	if (current >= '0'  && current <= '9') {
		out = (current - '0') << 4;
	}
	// a-f
	else if (current >= 'a'  && current <= 'f') {
		out = (current - 87) << 4;
	}
	// A-F
	else if (current >= 'A'  && current <= 'F') {
		out = (current - 55) << 4;
	}

	current = char1;
	// 0-9
	if (current >= '0'  && current <= '9') {
		out |= (current - '0');
	}
	// a-f
	if (current >= 'a'  && current <= 'f') {
		out |= (current - 87);
	}
	// A-F
	if (current >= 'A'  && current <= 'F') {
		out |= (current - 55);
	}

	return out;
}


e5_response_t* e5_packet_parse_response(uint8_t* rx, uint32_t len) {
  static e5_response_t ret = {.data = {}, .data_len = 0};
  ret.data_len = 0;

  const char* startLineToken = "+TEST: RX \"";
  const uint32_t strLenStartLineToken = strlen(startLineToken);

  /* check data avail and even number */
  if (len < strLenStartLineToken) {
      return NULL;
  }

  /* check if line start with rx token */
  if(strncmp((char*)rx, startLineToken, strLenStartLineToken) != 0){
      return NULL;
  }

  /* end token */
  const char* endPos = strstr((char*)rx, "\"\r\n");
  char* startOfStr = (char*)&rx[strLenStartLineToken];

  /* ensure end pos found, and strlen is modulo of 2 */
  if(!endPos || endPos < startOfStr || ((endPos - startOfStr) % 2 != 0)){
      return NULL;
  }

  /* parse data */
  for (char* i = startOfStr; i < endPos; i += 2) {
    ret.data[ret.data_len++] = privHexToString(*i, *(i+1));
  }

  return &ret;
}
