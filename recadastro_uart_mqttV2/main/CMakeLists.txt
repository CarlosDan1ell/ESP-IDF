idf_component_register(
    SRCS 
        "main.c"
        "../Drivers/driver_nvs.c"
        "../Drivers/driver_uart.c"
        "../Drivers/driver_WiFi.c"
        "../Drivers/driver_Mqtt.c"
        "../Drivers/driver_Mensagem.c"
    INCLUDE_DIRS 
        "." 
        "../Drivers"
)

