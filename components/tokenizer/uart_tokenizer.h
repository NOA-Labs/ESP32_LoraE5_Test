/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UARTTOKENIZER_H
#define __UARTTOKENIZER_H

#include "freertos/FreeRTOS.h"

typedef struct {
    uint8_t* data;
    uint32_t bytesRead;
    uint32_t sentenceIdent;
} stream_char_tokenizer_result_t;

typedef struct
{
    const uint8_t token;
    const uint32_t bufferMaxLen;
    
    uint8_t* data;
    uint32_t bytesCollected;
    uint8_t sentenceIdent;
    stream_char_tokenizer_result_t* result;
} stream_char_tokenizer_config_t;

typedef struct {
  const char* mnenomic;
  uint8_t strLen;
  uint8_t counter;
  uint8_t status;
} stream_mnemonic_t;

typedef struct
{
    stream_mnemonic_t startToken;
    stream_mnemonic_t endToken;
    const uint32_t bufferMaxLen;    
    uint8_t* data;
    uint32_t bytesCollected;
    stream_char_tokenizer_result_t* result;
} stream_mnemnic_tokenizer_config_t;

stream_char_tokenizer_result_t* stream_tokenize_by_char(stream_char_tokenizer_config_t* config, uint8_t in);
stream_char_tokenizer_result_t* stream_tokenize_by_mnemonic(stream_mnemnic_tokenizer_config_t* config, uint8_t in);

#endif