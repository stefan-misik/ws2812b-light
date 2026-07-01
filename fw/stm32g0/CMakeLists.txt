cmake_minimum_required(VERSION 3.24)

project(ws2812b_light_stm32g0 LANGUAGES C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(PROJECT_NAME_ELF ws2812b-light)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32G070KBTX_FLASH.ld)
set(VENDOR_STM ${CMAKE_CURRENT_SOURCE_DIR}/vendor/STM32CubeG0)
set(VENDOR_STM_CMSIS ${VENDOR_STM}/Drivers/CMSIS)
set(VENDOR_STM_DRIVER ${VENDOR_STM}/Drivers/STM32G0xx_HAL_Driver)
set(GENERATED_SHARED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/shared)
set(GENERATED_SONG_DIR ${GENERATED_SHARED_DIR}/songs)

set(STM32G0_SOURCES
    support/startup_stm32g070kbtx.s
    support/sysmem.c
    support/syscalls.c
    support/system_stm32g0xx.c
    tools/button_filter.cpp
    driver/base.cpp
    driver/common.cpp
    driver/systick.cpp
    driver/led_controller.cpp
    driver/ir_receiver.cpp
    driver/buzzer.cpp
    driver/keypad.cpp
    driver/status_leds.cpp
    driver/cpu_usage.cpp
    driver/i2c_bus.cpp
    driver/i2c_bus/cat24cx.cpp
    app/tools/color.cpp
    app/input/keypad.cpp
    app/input/ir_remote.cpp
    app/animation/tools/color_themes.cpp
    app/animation/color.cpp
    app/animation/rainbow.cpp
    app/animation/retro.cpp
    app/animation/twinkle.cpp
    app/animation/shifting_color.cpp
    app/animation/lights.cpp
    app/music.cpp
    app/animation_storage.cpp
    app/led_correction.cpp
    app/event_queue.cpp
    app/input.cpp
    app/io.cpp
    app/lights.cpp
    app/main.cpp
)

set(VENDOR_STM_DRIVER_LL
    rcc
    utils
    pwr
    gpio
    dma
    iwdg
    tim
    i2c
    usart
)

foreach(driver_name IN LISTS VENDOR_STM_DRIVER_LL)
    set(driver_source ${VENDOR_STM_DRIVER}/Src/stm32g0xx_ll_${driver_name}.c)
    if(EXISTS ${driver_source})
        list(APPEND STM32G0_SOURCES ${driver_source})
    endif()
endforeach()

set(SONGS
    silent_night
    jingle_bells
)

set(GENERATED_SONGS)
set(SONG_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../shared/songs/compose.py)
set(SONG_COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_SONG_DIR})

foreach(song_name IN LISTS SONGS)
    list(APPEND SONG_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../shared/songs/${song_name}.tune)
    list(APPEND GENERATED_SONGS ${GENERATED_SONG_DIR}/${song_name}.inc)
    list(APPEND SONG_COMMANDS
        COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/../shared/songs/compose.py
                ${CMAKE_CURRENT_SOURCE_DIR}/../shared/songs/${song_name}.tune
                -o ${GENERATED_SONG_DIR}/${song_name}.inc
    )
endforeach()

add_custom_command(
    OUTPUT ${GENERATED_SONGS}
    ${SONG_COMMANDS}
    DEPENDS ${SONG_DEPENDS}
    VERBATIM
)

add_custom_target(generate_songs DEPENDS ${GENERATED_SONGS})

add_executable(ws2812b_light_stm32g0 ${STM32G0_SOURCES})
add_dependencies(ws2812b_light_stm32g0 generate_songs)

set_target_properties(ws2812b_light_stm32g0 PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
)

target_include_directories(ws2812b_light_stm32g0 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${GENERATED_SHARED_DIR}
    ${VENDOR_STM_CMSIS}/Device/ST/STM32G0xx/Include
    ${VENDOR_STM_CMSIS}/Include
    ${VENDOR_STM_DRIVER}/Inc
)

target_compile_definitions(ws2812b_light_stm32g0 PRIVATE
    STM32
    STM32G0
    STM32G070KBTx
    STM32G070xx
    USE_FULL_LL_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

target_compile_options(ws2812b_light_stm32g0 PRIVATE
    -g3
    -Og
    -Wall
    -Wextra
    -Werror
    -ffunction-sections
    -fdata-sections
    $<$<COMPILE_LANGUAGE:C>:-std=c99>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-register>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>
)

target_link_libraries(ws2812b_light_stm32g0 PRIVATE c m)

target_link_options(ws2812b_light_stm32g0 PRIVATE
    -specs=nano.specs
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--cref
    -Wl,-Map,$<TARGET_FILE_DIR:ws2812b_light_stm32g0>/${PROJECT_NAME_ELF}.map
    $<$<CONFIG:Debug>:-Wl,--defsym=_Fw_Offset=64K>
)

set_target_properties(ws2812b_light_stm32g0 PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME_ELF}
    SUFFIX .elf
    LINK_DEPENDS ${LINKER_SCRIPT}
)

add_custom_command(
    TARGET ws2812b_light_stm32g0 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} --strip-all $<TARGET_FILE:ws2812b_light_stm32g0> -O ihex
            $<TARGET_FILE_DIR:ws2812b_light_stm32g0>/${PROJECT_NAME_ELF}.hex
    VERBATIM
)

if(CMAKE_SIZE)
    add_custom_command(
        TARGET ws2812b_light_stm32g0 POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:ws2812b_light_stm32g0>
        VERBATIM
    )
endif()
