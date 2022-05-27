#include "uart_tokenizer.h"
#include <string.h>

uint8_t privGetMatch(stream_mnemonic_t* token, uint8_t in) {
  if (in == token->mnenomic[token->counter]) {  // char match
    token->counter++;

    if (token->counter == token->strLen) {  // full match
      token->counter = 0;
      token->status = 1;
      return 1;
    }
  } else {
    token->counter = 0;  // match interrupted, reset
  }

  return 0;
}

stream_char_tokenizer_result_t* stream_tokenize_by_mnemonic(stream_mnemnic_tokenizer_config_t* config, uint8_t in) {
  stream_char_tokenizer_result_t* ret = NULL;

  if (!privGetMatch(&config->endToken, in)) {
    // Do not add the last char of the start token
    // The next iteration will know (startToken.status) that the token has been received
    if (privGetMatch(&config->startToken, in)) {
      return ret;
    }

    if (config->startToken.status && config->bytesCollected < config->bufferMaxLen) {
      config->data[config->bytesCollected++] = in;
    }
  } else {
    if (config->bytesCollected >= (config->endToken.strLen - 1)) {
      config->bytesCollected -= (config->endToken.strLen - 1);
    }

    config->data[config->bytesCollected] = 0x00;

    /* add to result */
    config->result->bytesRead = config->bytesCollected;
    config->result->data = config->data;
    ret = config->result;

    /* restart */
    config->bytesCollected = 0;
    config->startToken.status = 0;
  }

  return ret;
}
