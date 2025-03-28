##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.7.1] date: [Thu Jul 23 00:07:34 BST 2020]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = uCNC
# USE_USB = TRUE

######################################
# building variables
######################################
# ifneq "$(findstring debug, $(MAKECMDGOALS))" ""
# DEBUG = 1
# endif
# debug build?
DEBUG ?= 0
# optimization
ifeq ($(DEBUG), 1)
OPT = -Og
else
OPT = -Os
endif


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# helper
######################################
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

######################################
# source
######################################
# C sources
SRC_FILES = $(call rwildcard,./,*.c)
SRC_FILES += $(call rwildcard,../../uCNC,*.c)
C_SOURCES = $(filter-out $(call rwildcard,../../uCNC/src/tinyusb,*.c), $(SRC_FILES))

#######################################
# binaries
#######################################
PREFIX = avr-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -j .text -j .data -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU ?= atmega328p
FREQ ?= 16000000L
BOARD ?= BOARD_UNO

# mcu
MCU = -mmcu=$(CPU)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
BUILD_OPTIONS ?=

C_DEFS =  $(BUILD_OPTIONS)\
-DBOARD=$(BOARD)

# AS includes
AS_INCLUDES = 

# C includes
HDR_FILES = $(call rwildcard,./,*.h)
C_HEADERS = $(filter-out $(call rwildcard,../../uCNC/src/tinyusb,*.h), $(HDR_FILES))

# C includes
C_INCLUDES =  \
-I"./" \
-I"../../uCNC/" \
-I"../../uCNC/src"

C_INCLUDES += $(foreach d,$(C_HEADERS),$(addsuffix $(dir $(d)),"-I"))

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Wextra -gdwarf-2 -flto -fuse-linker-plugin -Wl,--gc-sections,--relax

CFLAGS = $(MCU) -DF_CPU=$(FREQ) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wextra -std=gnu99 -ffunction-sections -fdata-sections -mcall-prologues -mrelax -flto -fno-fat-lto-objects -fno-tree-scev-cprop

ifeq ($(DEBUG), 1)
CFLAGS += -g3 -ggdb3 -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# default action: build all
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(ASFLAGS) -o $@
	$(SZ) -C --mcu=$(CPU) $@

#  $(BUILD_DIR)/$(TARGET).elf: $(C_SOURCES)
# 	$(CC) -fwhole-program $(CFLAGS) $(ASFLAGS) $^ -o $@
# 	$(SZ) -C --mcu=$(CPU) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***