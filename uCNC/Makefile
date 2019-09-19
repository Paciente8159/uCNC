# Project: ucnc
# Makefile created by Dev-C++ 5.11

CPP      = avr-g++.exe
CC       = avr-gcc.exe
CPU		 = atmega328
FREQ     = 16000000UL
OPTFLAG  = -O0
WINDRES  = 
OBJ      = main.o board_uno.o structures.o cnc.o motion.o protocol.o
LINKOBJ  = main.o board_uno.o structures.o cnc.o motion.o protocol.o
LIBS     = -w $(OPTFLAG) -g -flto -fuse-linker-plugin -Wl,--gc-sections -mmcu=$(CPU)
INCS     = -I"K:/Dev-Cpp/WinAVR/avr/include"
CXXINCS  = -I"K:/Dev-Cpp/WinAVR/avr/include"
BIN      = ucnc.elf
CXXFLAGS = $(CXXINCS) -c -g $(OPTFLAG) -w -std=gnu11 -ffunction-sections -fdata-sections -MMD -flto -fno-fat-lto-objects -mmcu=$(CPU) -DF_CPU=$(FREQ)
CFLAGS   = $(INCS) -c -g $(OPTFLAG) -w -std=gnu11 -ffunction-sections -fdata-sections -MMD -flto -fno-fat-lto-objects -mmcu=$(CPU) -DF_CPU=$(FREQ)
RM       = rm.exe -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN) *.d

$(BIN): $(OBJ)
	$(CC) $(LIBS) $(LINKOBJ) -o $(BIN)
	${RM} $(OBJ) *.d

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

	
