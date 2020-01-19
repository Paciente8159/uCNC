# Project: uCNC

MCU 	 = virtual
CC       = gcc.exe
SOURCE   = main.c settings.c kinematics_cartesian_xyz.c kinematics_corexy.c planner.c cnc.c parser.c protocol.c motion_control.c serial.c dio_control.c interpolator_linear.c
LIBS     = -L"" -static-libgcc -g3
INCS     = -I""
CFLAGS   = $(INCS) -Og -std=gnu99 -g3 -DMCU=MCU_VIRTUAL -D__SIMUL__ -D__DEBUG__
BIN      = $(BUILDDIR)/uCNC.exe
SOURCEDIR= ../..
BUILDDIR = build
SRC 	 = $(addprefix $(SOURCEDIR)/,$(SOURCE))
OBJ 	 = $(addprefix $(BUILDDIR)/,$(SOURCE:.c=.o) mcu.o)
DOBJ 	 = $(OBJ:.o=.d)
LINKOBJ  = $(OBJ)
RM       = rm.exe -f



.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(DOBJ) $(BIN)

$(BIN): $(LINKOBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILDDIR)/mcu.o: mcu_atmega328.c
	$(CC) -c $< -o $@ $(CFLAGS)
