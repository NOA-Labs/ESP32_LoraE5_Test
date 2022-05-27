set(srcs    "FreeRTOS_CLI.c"
            )

idf_component_register(SRCS      "${srcs}"
                    INCLUDE_DIRS ".")
