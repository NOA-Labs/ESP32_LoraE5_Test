set(srcs    "e5_packet_parser.c"
            "lora_icq.c"
            "lora.c"
            "bsp_e5_drv.c")

idf_component_register(SRCS         "${srcs}"
                        INCLUDE_DIRS    "."
                        PRIV_REQUIRES hardware_drv tokenizer
                    )
