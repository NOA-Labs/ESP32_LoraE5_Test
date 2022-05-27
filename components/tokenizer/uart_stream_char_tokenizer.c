#include "uart_tokenizer.h"
#include <string.h>

stream_char_tokenizer_result_t* stream_tokenize_by_char(stream_char_tokenizer_config_t* config, uint8_t in){
    
    stream_char_tokenizer_result_t* ret = NULL;

    if(in != config->token){
        
        /* collect bytes until token is reached */
        if(config->bytesCollected < config->bufferMaxLen){
            config->data[config->bytesCollected++] = in;

            /* simple hash for identifier */
            if(config->bytesCollected<=6){config->sentenceIdent += in;}
        }
    }else{
        
        /* terminate */
        config->data[config->bytesCollected++] = config->token;
        config->data[config->bytesCollected] = 0x00;

        /* add to result */
        config->result->bytesRead = config->bytesCollected;
        config->result->data = config->data;
        config->result->sentenceIdent = config->sentenceIdent;
        ret = config->result;

        /* restart */
        config->bytesCollected = 0;
        config->sentenceIdent = 0xff;
    }

    return ret;
}