LIBMARIO_INPUT_FILES := ./mariolib/funcs.c 
DECOMP_INPUT_FILES := ../src/game/mario_actions_airborne.c ../src/game/mario_actions_automatic.c ../src/game/mario_actions_cutscene.c \
	../src/game/mario_actions_moving.c ../src/game/mario_actions_object.c ../src/game/mario_actions_stationary.c ../src/game/mario_actions_submerged.c \
	../src/game/mario_misc.c ../src/game/mario_step.c ../src/game/mario.c ../src/engine/math_util.c ../src/game/object_helpers.c

ALL_INPUT_FILES = $(LIBMARIO_INPUT_FILES) $(DECOMP_INPUT_FILES)


LIBMARIO_OUTPUT_FILES := $(foreach f,$(LIBMARIO_INPUT_FILES:.c=.o),build/libmario/$(notdir $f))
DECOMP_OUTPUT_FILES := $(foreach f,$(DECOMP_INPUT_FILES:.c=.o),build/src/$(notdir $f))

ALL_OUTPUT_FILES = $(LIBMARIO_OUTPUT_FILES) $(DECOMP_OUTPUT_FILES)

all: libmario.o

CROSS ?= mips-linux-gnu-

#AS		  := $(CROSS)as
CC        := $(CROSS)gcc
CPP       := cpp -P
LD        := $(CROSS)ld
AR        := $(CROSS)ar
OBJDUMP    := $(CROSS)objdump

OBJDUMP_FLAGS := -d -r -z -Mreg-names=32

DEFINES = -DTARGET_N64 -D_LANGUAGE_C -DNON_MATCHING -DAVOID_UB

OPT_FLAGS = -Os -ffast-math

SHARING = -mno-shared -fno-common

CFLAGS = -nostdinc -G 0 -mips3 $(SHARING) -march=vr4300 -mfix4300 -mabi=32 -mhard-float \
	-mdivide-breaks -fno-stack-protector -fno-zero-initialized-in-bss \
	-fno-PIC -mno-abicalls -fno-strict-aliasing -fno-inline-functions -ffreestanding \
	-fwrapv -Wno-missing-braces

INCLUDES = -I.. -I../include -I../include/n64 -I../include/libc -I../src/engine -I../src/game -I../src -Ism64

LIBS = -L$(N64_LIBGCCDIR) -lgcc -L../lib -L../lib/n64 -lultra_d

$(shell mkdir -p build build/libmario build/src)

clean:
	rm -rf build/*
	rm -f libmario.o
	rm -f libmario.a
	rm -f output.map

build/libmario/%.o: mariolib/%.c
	$(CC) -c $(DEFINES) $(OPT_FLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<

build/src/%.o: ../src/game/%.c
	$(CC) -c $(DEFINES) $(OPT_FLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<

build/src/%.o: ../src/engine/%.c
	$(CC) -c $(DEFINES) $(OPT_FLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<

libmario.o: $(LIBMARIO_OUTPUT_FILES) $(DECOMP_OUTPUT_FILES)
	$(LD) -r -o $@ $(ALL_OUTPUT_FILES) -Map=output.map

libmario.a: libmario.o
	$(AR) rcs -o $@ libmario.o
