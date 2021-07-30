# Project: µCNC

MCU 	 = atmega328p
CC       = avr-gcc
LIBS     = -w -Os -gdwarf-2 -flto -fuse-linker-plugin -Wl,--gc-sections,--relax -mmcu=$(MCU)
CFLAGS   = -I"../../src/" -Os -Wall -Wextra -gdwarf-2 -w -std=gnu99 -ffunction-sections -fdata-sections -mcall-prologues -mrelax -MMD -flto -fno-fat-lto-objects -mmcu=$(MCU) -DF_CPU=16000000L
BIN      = $(BUILDDIR)/uCNC.elf
HEX      = $(BIN:.elf=.hex)
SOURCEDIR= ../../src
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
SOURCE   = $(wildcard *.c)\
		   $(call rwildcard,$(SOURCEDIR)/,*.c)
		   
BUILDDIR = build
OBJ 	 = $(SOURCE:.c=.o)
LINKOBJ  = $(addprefix $(BUILDDIR)/,$(notdir $(OBJ)))
DOBJ 	 = $(LINKOBJ:.o=.d)
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-rm -fR $(BUILDDIR)

all-before:
	-mkdir build

$(OBJ): %.o: %.c
	$(CC) -c $< -o $(BUILDDIR)/$(@F) $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

all-after: $(BIN)
	${RM} $(HEX)
	avr-objcopy -j .text -j .data -O ihex $(BIN) $(HEX)
	avr-size -C --mcu=$(MCU) $(BIN)
	
