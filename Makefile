# Direct Makefile for the LAME encoder tree.

CC ?= cc
AR ?= ar
RANLIB ?= ranlib
RM ?= rm -f

UNAME := $(shell uname -s)

CPPFLAGS += -DHAVE_CONFIG_H -I. -Iinclude -Ifrontend -Ilibmp3lame -Impglib
CFLAGS ?= -O3 -Wall -fno-common
LDFLAGS ?=
LDLIBS := -lm

ifneq ($(OS),Windows_NT)
LDLIBS := -lncurses $(LDLIBS)
endif

ifeq ($(OS),Windows_NT)
else ifeq ($(UNAME),Darwin)
LDLIBS += -liconv
endif

FRONTEND := frontend/lame
MP3LIB := libmp3lame/libmp3lame.a
MPGLIB := mpglib/libmpgdecoder.a

FRONTEND_SRCS := \
	frontend/lame_main.c \
	frontend/main.c \
	frontend/brhist.c \
	frontend/console.c \
	frontend/get_audio.c \
	frontend/lametime.c \
	frontend/parse.c \
	frontend/timestatus.c

LIBMP3LAME_SRCS := \
	libmp3lame/VbrTag.c \
	libmp3lame/bitstream.c \
	libmp3lame/encoder.c \
	libmp3lame/fft.c \
	libmp3lame/gain_analysis.c \
	libmp3lame/id3tag.c \
	libmp3lame/lame.c \
	libmp3lame/newmdct.c \
	libmp3lame/presets.c \
	libmp3lame/psymodel.c \
	libmp3lame/quantize.c \
	libmp3lame/quantize_pvt.c \
	libmp3lame/reservoir.c \
	libmp3lame/set_get.c \
	libmp3lame/tables.c \
	libmp3lame/takehiro.c \
	libmp3lame/util.c \
	libmp3lame/vbrquantize.c \
	libmp3lame/version.c \
	libmp3lame/mpglib_interface.c

MPGLIB_SRCS := \
	mpglib/common.c \
	mpglib/dct64_i386.c \
	mpglib/decode_i386.c \
	mpglib/interface.c \
	mpglib/layer1.c \
	mpglib/layer2.c \
	mpglib/layer3.c \
	mpglib/tabinit.c

FRONTEND_OBJS := $(FRONTEND_SRCS:.c=.o)
LIBMP3LAME_OBJS := $(LIBMP3LAME_SRCS:.c=.o)
MPGLIB_OBJS := $(MPGLIB_SRCS:.c=.o)
OBJS := $(FRONTEND_OBJS) $(LIBMP3LAME_OBJS) $(MPGLIB_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean test

all: $(FRONTEND)

$(FRONTEND): $(FRONTEND_OBJS) $(MP3LIB) $(MPGLIB)
	$(CC) $(LDFLAGS) -o $@ $(FRONTEND_OBJS) $(MP3LIB) $(MPGLIB) $(LDLIBS)

$(MP3LIB): $(LIBMP3LAME_OBJS)
	$(AR) cr $@ $(LIBMP3LAME_OBJS)
	$(RANLIB) $@

$(MPGLIB): $(MPGLIB_OBJS)
	$(AR) cr $@ $(MPGLIB_OBJS)
	$(RANLIB) $@

%.o: %.c config.h Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: $(FRONTEND)
	$(FRONTEND) --nores testcase.wav testcase.new.mp3
	@echo
	@echo "The following output has value only for a LAME-developer, do not make _any_"
	@echo "assumptions about what this number means. You do not need to care about it."
	@cmp -l testcase.new.mp3 testcase.mp3 | wc -l

clean:
	$(RM) $(FRONTEND) $(MP3LIB) $(MPGLIB) $(OBJS) $(DEPS) testcase.new.mp3

-include $(DEPS)
