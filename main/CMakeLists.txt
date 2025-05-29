
idf_component_register(
    SRCS 
        "main.c" 
        "SD_Card/SD_SPI.c"
        "pngle/pngle.c"
        "pngle/miniz.c"

    INCLUDE_DIRS 
        "./SD_Card"
        "./pngle"
        "."
)
