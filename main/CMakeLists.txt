file(GLOB MAIN_SRCS
    "*.c"
    "SD_Card/*.c"
    "st7789/*.c"
    "tinybit/*.c"
    "tinybit/lua/*.c"
    "tinybit/pngle/*.c"
)

idf_component_register(
    SRCS ${MAIN_SRCS}
    INCLUDE_DIRS 
        "./SD_Card"
        "./st7789"
        "."
        "tinybit"
        "tinybit/lua"
        "tinybit/pngle"

)