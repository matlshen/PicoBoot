cmake_minimum_required(VERSION 3.5)

########################################################################
# Toolchain file must go at the top
########################################################################
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/arm-cm4.cmake)
# Choose between Debug and Release builds
set(CMAKE_BUILD_TYPE Debug)

########################################################################
# MCU information
# Edit this section based on the target platform
########################################################################
set(MCU_FAMILY STM32L4xx)
set(MCU_MODEL STM32L496xx)
set(MCU_PART STM32L496RGTx)
string(TOLOWER ${MCU_FAMILY} MCU_FAMILY_LOWER)
string(TOLOWER ${MCU_MODEL} MCU_MODEL_LOWER)

########################################################################
# Directory information
########################################################################
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../common)
set(BOOTUTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../bootutil)

########################################################################
# Includes
########################################################################
include_directories(
    Core/Inc
    USB_DEVICE/App
    USB_DEVICE/Target
    STM32_USB_Device_Library/Core/Inc
    STM32_USB_Device_Library/Class/CDC/Inc
    ${COMMON_DIR}/${MCU_FAMILY}_HAL_Driver/Inc
    ${COMMON_DIR}/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
    ${COMMON_DIR}/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${COMMON_DIR}/CMSIS/Include
)

########################################################################
# Sources
########################################################################
file(GLOB PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.s
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/App/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/Target/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32_USB_Device_Library/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32_USB_Device_Library/Class/CDC/Src/*.c
    ${COMMON_DIR}/${MCU_FAMILY}_HAL_Driver/Src/*.c
)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX ".*template.*")

# ASM source files
set(ASM_SOURCES
    Core/Src/startup_${MCU_MODEL_LOWER}.s
)

########################################################################
# Executable build instructions
########################################################################
# Project name is the name of the source directory
get_filename_component(SOURCE_DIR_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOURCE_DIR_NAME})

# Add bootutil library
add_subdirectory(../../../bootutil ${CMAKE_CURRENT_BINARY_DIR}/bootutil)

# Add executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${ASM_SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC
    ${MCU_MODEL}
    USE_HAL_DRIVER
    USE_FULL_LL_DRIVER)
target_compile_definitions(bootlib PUBLIC
    ${MCU_MODEL}
    USE_HAL_DRIVER
    USE_FULL_LL_DRIVER)

enable_language(ASM C)

# Linker script and libraries
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} \
    -specs=nano.specs \
    -T${CMAKE_CURRENT_SOURCE_DIR}/${MCU_PART}_FLASH.ld \
    -Wl,--gc-sections \
    -Wl,-Map=${PROJECT_NAME}.map,--cref \
    -Wl,--print-memory-usage"
)
target_link_libraries(${PROJECT_NAME} c m nosys bootlib)

# Post-build commands for generating binary and hex files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
)