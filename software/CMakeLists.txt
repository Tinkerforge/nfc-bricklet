CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake_config_custom.txt)

SET(PROJECT_NAME nfc-bricklet)
SET(CHIP_FAMILY XMC1)
SET(MCU cortex-m0\ -mthumb)
PROJECT(${PROJECT_NAME})

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
#SET(OPTIMIZATION_LEVEL 0)
SET(DEBUG "-g -ggdb")

SET(OPTIMIZATION_LEVEL 2)
#SET(DEBUG "")

SET(CMAKE_BUILD_TYPE None)
ENABLE_LANGUAGE(C ASM)

INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/src/"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/inc/"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/CMSIS/Include/"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/CMSIS/Infineon/XMC1300_series/Include/"
	
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/inc/"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/inc/"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NxpNci/inc/"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/xmc_port/"
)

# find source files
SET(SOURCES
	"${PROJECT_SOURCE_DIR}/src/main.c"
	"${PROJECT_SOURCE_DIR}/src/communication.c"
	"${PROJECT_SOURCE_DIR}/src/pn7150.c"
	"${PROJECT_SOURCE_DIR}/src/pn7150_reader.c"
	"${PROJECT_SOURCE_DIR}/src/pn7150_cardemu.c"
	"${PROJECT_SOURCE_DIR}/src/pn7150_p2p.c"
	"${PROJECT_SOURCE_DIR}/src/pn7150_simple.c"
	
	"${PROJECT_SOURCE_DIR}/src/bricklib2/hal/uartbb/uartbb.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/hal/system_timer/system_timer.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/hal/startup/startup_xmc1.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/hal/startup/startup_xmc1300.S"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/os/coop_task.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/os/coop_task_pendsv_handler.S"
	
	"${PROJECT_SOURCE_DIR}/src/bricklib2/bootloader/bootloader.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/bootloader/bootloader_xmc_irq.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/protocols/tfp/tfp.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/utility/ringbuffer.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/utility/pearson_hash.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/utility/communication_callback.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/utility/crc16.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/utility/led_flicker.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/logging/logging.c"

	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc_gpio.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc1_gpio.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc_spi.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc_usic.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc1_scu.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc1_flash.c"
	"${PROJECT_SOURCE_DIR}/src/bricklib2/xmclib/XMCLib/src/xmc_i2c.c"
	
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/P2P_NDEF.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/RW_NDEF_T1T.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/RW_NDEF_T2T.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/RW_NDEF_T3T.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/RW_NDEF_T4T.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/RW_NDEF.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NdefLibrary/src/T4T_NDEF_emu.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/NxpNci/src/NxpNci.c"
	"${PROJECT_SOURCE_DIR}/src/NfcLibrary/xmc_port/xmc_port.c"
)

MESSAGE(STATUS "\nFound following source files:\n ${SOURCES}\n")

# define executable
include(${CMAKE_CURRENT_SOURCE_DIR}/src/bricklib2/cmake/configs/config_define_executable.txt)

# add aeabi aeabi_idiv, aeabi_uidiv, aeabi_idivmod and aeabi_uidivmod
include(${CMAKE_CURRENT_SOURCE_DIR}/src/bricklib2/cmake/configs/config_comcu_add_aeabi_libraries.txt)

# define standard compile and linker flags
SET(LINKER_SCRIPT_NAME xmc1_firmware_with_brickletboot.ld)
SET(FLASH_ORIGIN 0x10003000) # Move flash origin above the bootloader
SET(FLASH_EEPROM_LENGTH 1024) # Flash used for EEPROM emulation at end of flash (multiple of page size (256 byte))
MATH(EXPR FLASH_LENGTH "${CHIP_FLASH_SIZE} - 8192 - ${FLASH_EEPROM_LENGTH}") # Remove bootloader size from flash size
include(${CMAKE_CURRENT_SOURCE_DIR}/src/bricklib2/cmake/configs/config_comcu_add_standard_flags.txt)

# add custom build commands
include(${CMAKE_CURRENT_SOURCE_DIR}/src/bricklib2/cmake/configs/config_comcu_add_standard_custom_commands.txt)

# add preprocessor defines
include(${CMAKE_CURRENT_SOURCE_DIR}/src/bricklib2/cmake/configs/config_xmc1_add_preprocessor_defines.txt)

ADD_DEFINITIONS(-DRW_SUPPORT -DP2P_SUPPORT -DCARDEMU_SUPPORT)
