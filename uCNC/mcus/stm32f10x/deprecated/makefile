# Project: µCNC

MCU 	 = cortex-m3
CC       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
C_INCLUDES =  \
-IInc \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include \
-I../..
LIBS     = -T STM32F103C8T6.ld -w -Os -flto -fuse-linker-plugin -lm --specs=nano.specs -Wl,--gc-sections -mcpu=$(MCU)
CFLAGS   = -DBOARD=BOARD_BLUEPILL $(C_INCLUDES) -Wall -g -std=gnu99 -Os -mthumb -mcpu=$(MCU) -mfloat-abi=soft -mlittle-endian -ffunction-sections -fdata-sections -Wstrict-prototypes -Warray-bounds -Wno-return-type -fno-strict-aliasing -Wno-unused-const-variable --specs=nano.specs -DSTM32F10X_MD
ELF      = $(BUILDDIR)/uCNC.elf
BIN      = $(ELF:.elf=.bin)
HEX      = $(ELF:.elf=.hex)
SOURCEDIR= ../..
SOURCE   = $(wildcard *.c) \
		   $(wildcard */*.c) \
		   $(wildcard $(SOURCEDIR)/*.c)
BUILDDIR = build
OBJ 	 = $(SOURCE:.c=.o)
LINKOBJ  = $(addprefix $(BUILDDIR)/,$(notdir $(OBJ)))
DOBJ 	 = $(LINKOBJ:.o=.d)
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(ELF) all-after

clean: clean-custom
	${RM} $(OBJ) $(DOBJ) $(ELF) $(BIN) $(HEX)

all-before:
	-mkdir build

$(OBJ): %.o: %.c
	$(CC) -c $< -o $(BUILDDIR)/$(@F) $(CFLAGS)

$(ELF): $(OBJ)
	$(CC) $(LINKOBJ) -o $(ELF) $(LIBS)

all-after: $(ELF)
	${RM} $(BIN)
	${RM} $(HEX)
	$(OBJCOPY) -O ihex $(ELF) $(HEX)
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	arm-none-eabi-nm $(ELF)
	arm-none-eabi-objdump.exe -D $(ELF) > $(ELF).asm
	arm-none-eabi-size -G $(ELF)

	