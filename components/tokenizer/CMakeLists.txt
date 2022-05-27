set(srcs    "uart_stream_char_tokenizer.c"
            "uart_stream_mnemonic_tokenizer.c"
            )

idf_component_register(SRCS         "${srcs}"
                    INCLUDE_DIRS    "."
                                    )
