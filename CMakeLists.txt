cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

# Define o nome do projeto como Teste_ADC_Display, suportando C, C++ e Assembly.
project(Tarefa2_Fase2 C CXX ASM)
pico_sdk_init()

add_executable(
    Tarefa2_Fase2 Tarefa2_Fase2.c 
    lib/ssd1306.c
    )


target_compile_definitions(Tarefa2_Fase2 PRIVATE 
    PICO_PRINTF_SUPPORT_FLOAT=1 
    PICO_STDIO_ENABLE_PRINTF=1
    )

target_link_libraries(Tarefa2_Fase2
        pico_stdlib
        hardware_i2c
        hardware_adc
        hardware_irq
        )

pico_enable_stdio_uart(Tarefa2_Fase2 0)
pico_enable_stdio_usb(Tarefa2_Fase2 1)

pico_add_extra_outputs(Tarefa2_Fase2)

