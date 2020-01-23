# Project: uCNC

MCU 	 = atmega328p
CC       = avr-gcc.exe
SOURCE   = main.c settings.c kinematics_cartesian_xyz.c kinematics_corexy.c planner.c cnc.c parser.c protocol.c motion_control.c serial.c io_control.c interpolator.c
LIBS     = -w -Os -gdwarf-2 -flto -fuse-linker-plugin -Wl,--gc-sections -mmcu=$(MCU)
CFLAGS   = -Os -Wall -Wextra -D__DEBUG__ -Os -gdwarf-2 -w -std=gnu11 -ffunction-sections -fdata-sections -MMD -flto -fno-fat-lto-objects -mmcu=$(MCU) -DF_CPU=16000000L -DMCU=MCU_ATMEGA328P
BIN      = $(BUILDDIR)/uCNC.elf
HEX      = $(BIN:.elf=.hex)
SOURCEDIR= ../../..
BUILDDIR = build
SRC 	 = $(addprefix $(SOURCEDIR)/,$(SOURCE))
OBJ 	 = $(addprefix $(BUILDDIR)/,$(SOURCE:.c=.o) mcu.o)
DOBJ 	 = $(OBJ:.o=.d)
LINKOBJ  = $(OBJ)
RM       = rm.exe -f



.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(DOBJ) $(BIN) $(HEX)

$(BIN): $(LINKOBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILDDIR)/mcu.o: mcu_atmega328.c
	$(CC) -c $< -o $@ $(CFLAGS)

all-after: $(BIN)
	${RM} $(HEX)
	avr-objcopy -j .text -j .data -O ihex $(BIN) $(HEX)
	avr-size -C --mcu=$(MCU) $(BIN)