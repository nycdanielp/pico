# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Force fetch SDK v2.0+ from git because v1.5.1 doesn't support Pico 2 W
set(ENV{PICO_SDK_PATH} "")
set(PICO_SDK_FETCH_FROM_GIT OFF CACHE BOOL "" FORCE)
set(PICO_SDK_FETCH_FROM_GIT_TAG "a1438df" CACHE STRING "" FORCE)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk/pico_sdk-src CACHE PATH "" FORCE)
set(PICO_SDK_FETCH_FROM_GIT_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk CACHE FILEPATH "" FORCE)
set(PICO_BOARD pico2_w CACHE STRING "Board type" FORCE)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

include(pico_sdk_import.cmake)

project(nano_os C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Define FatFs path based on the downloaded SDK
set(TUSB_FATFS_DIR ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source)

add_executable(nano_os
    main.c
    kernel.c
    tcp_server.c
    cli.c
    usb_fs.c  # Enabled for rp2350 compatibility
    config_store.c
    # Compile FatFs library directly into the OS
    ${TUSB_FATFS_DIR}/ff.c
    ${TUSB_FATFS_DIR}/ffsystem.c
    ${TUSB_FATFS_DIR}/ffunicode.c
)

# Embed wifi.txt as binary data
pico_embed_file(nano_os wifi.txt)

# Disable USB device stdio because the USB port is now a Host
pico_enable_stdio_usb(nano_os 0)
pico_enable_stdio_uart(nano_os 0)

# Add our project directory to include paths
target_include_directories(nano_os PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TUSB_FATFS_DIR}
)

target_link_libraries(nano_os
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_gpio
    hardware_flash
    hardware_sync
    # Link TinyUSB Host and Board support
    tinyusb_host
    tinyusb_board
)

# Bypass host build tools which fail without a Windows C++ compiler
pico_add_extra_outputs(nano_os)
