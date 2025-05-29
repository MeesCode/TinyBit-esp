file(GLOB MAIN_SRCS
    "*.c"
    "pngle/*.c"
    "SD_Card/*.c"
    "tinybit/*.c"
    "tinybit/lua/*.c"
)

idf_component_register(
    SRCS ${MAIN_SRCS}
    INCLUDE_DIRS 
        "./SD_Card"
        "./pngle"
        "."
        "tinybit"
        "tinybit/lua"
)