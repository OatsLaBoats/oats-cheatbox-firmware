cmake_minimum_required(VERSION 3.16.3)

include(pico_sdk_import.cmake)

project(oats-cheatbox-firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.c
    src/platform/usb_descriptors.c
    src/platform/platform.c
    src/profile.c
    src/virtual_button.c

    src/profiles/default.c
    src/profiles/ggst.c
    src/profiles/ggxrd.c
)

pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    tinyusb_device
    tinyusb_board
)