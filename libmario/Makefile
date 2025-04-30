LIBMARIO_INPUT_FILES := ./mariolib/funcs.c ./mariolib/model_data.c
LIBMARIO_DECOMP_INPUT_FILES := ../src/game/mario_actions_airborne.c ../src/game/mario_actions_automatic.c ../src/game/mario_actions_cutscene.c \
	../src/game/mario_actions_moving.c ../src/game/mario_actions_object.c ../src/game/mario_actions_stationary.c ../src/game/mario_actions_submerged.c \
	../src/game/mario_misc.c ../src/game/mario_step.c ../src/game/mario.c ../src/engine/math_util.c ../src/engine/geo_layout.c  ../src/engine/graph_node.c ../src/game/object_helpers.c ../src/game/rendering_graph_node.c \
	../src/engine/graph_node_manager.c

ALL_LIBMARIO_INPUT_FILES = $(LIBMARIO_INPUT_FILES) $(LIBMARIO_DECOMP_INPUT_FILES)


LIBMARIO_OUTPUT_FILES := $(foreach f,$(LIBMARIO_INPUT_FILES:.c=.o),build/libmario/$(notdir $f))
DECOMP_OUTPUT_FILES := $(foreach f,$(LIBMARIO_DECOMP_INPUT_FILES:.c=.o),build/src/$(notdir $f))

ALL_OUTPUT_FILES = $(LIBMARIO_OUTPUT_FILES) $(DECOMP_OUTPUT_FILES)

all: libmario.o

CC        := clang
LD        := ld.lld

DEFINES = -DTARGET_N64 -D_LANGUAGE_C -DNON_MATCHING -DAVOID_UB -DF3DEX_GBI_2 -DF3DEX_GBI_PL -DGBI_DOWHILE

OPT_FLAGS = -O2 -ffast-math

SHARING = 

CFLAGS = $(SHARING) -target mips -mips2 -mabi=32 $(OPT_FLAGS) -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
	-fomit-frame-pointer -fno-unsafe-math-optimizations -fno-builtin-memset \
	-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter -Wno-unknown-pragmas -Wno-unused-variable \
	-Wno-missing-braces -Wno-unsupported-floating-point-opt -Werror=section -nostdinc -ffreestanding -fno-common

INCLUDES = -I.. -I../include -I../include/n64 -I../include/libc -I../src/engine -I../src/game -I../src -Ism64

BUILD_DIR := build
BUILD_DIRS := $(BUILD_DIR) $(BUILD_DIR)/src $(BUILD_DIR)/libmario

$(BUILD_DIRS):
ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$@)
else
	mkdir -p $@
endif

$(shell mkdir -p build build/libmario build/src)

clean:
	rm -rf build/*
	rm -f libmario.o
	rm -f libmario.a
	rm -f output.map

build/libmario/%.o: mariolib/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) $< -MMD -MF $(@:.o=.d)  -c -o $@

build/src/%.o: ../src/game/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) $< -MMD -MF $(@:.o=.d)  -c -o $@

build/src/%.o: ../src/engine/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) $< -MMD -MF $(@:.o=.d)  -c -o $@

libmario.o: $(ALL_OUTPUT_FILES) | $(BUILD_DIRS)
	$(LD) -r -o $@ $(ALL_OUTPUT_FILES) -Map=output.map
