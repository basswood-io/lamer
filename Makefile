# Direct Makefile for the LAME encoder tree.

CC ?= cc
AR ?= ar
RANLIB ?= ranlib
RM ?= rm -f
MKDIR_P ?= mkdir -p
INSTALL_DATA ?= cp -f
PREFIX ?= /usr/local
DESTDIR ?=
VERSION ?= 3.101.0

UNAME := $(shell uname -s)
AR_VERSION := $(shell $(AR) --version 2>/dev/null)

CPPFLAGS += -DHAVE_CONFIG_H -I. -Iinclude -Ifrontend -Ilibmp3lame
FRONTEND_CPPFLAGS :=
override CFLAGS += -O3 -Wall -fno-common
LDFLAGS ?=
LDLIBS := -lm
DEFAULT_ARFLAGS := $(if $(filter GNU LLVM,$(firstword $(AR_VERSION))),crD,cr)

ifneq ($(filter default undefined,$(origin ARFLAGS)),)
ARFLAGS := $(DEFAULT_ARFLAGS)
endif

ifeq ($(PIC),1)
override CFLAGS += -fPIC
endif

ifneq ($(OS),Windows_NT)
FRONTEND_CPPFLAGS += -DHAVE_ICONV -DHAVE_TERMCAP -DHAVE_TERMCAP_H
LDLIBS := -lncurses $(LDLIBS)
endif

ifeq ($(OS),Windows_NT)
else ifeq ($(UNAME),Darwin)
LDLIBS += -liconv
endif

FRONTEND := frontend/lame
MP3LIB := libmp3lame/libmp3lame.a

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
	libmp3lame/vector/xmm_quantize_sub.c \
	libmp3lame/vbrquantize.c \
	libmp3lame/version.c

FRONTEND_OBJS := $(FRONTEND_SRCS:.c=.o)
LIBMP3LAME_OBJS := $(LIBMP3LAME_SRCS:.c=.o)
OBJS := $(FRONTEND_OBJS) $(LIBMP3LAME_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean install lib test

all: $(FRONTEND)

lib: $(MP3LIB)

$(FRONTEND_OBJS): CPPFLAGS += $(FRONTEND_CPPFLAGS)

$(FRONTEND): $(FRONTEND_OBJS) $(MP3LIB)
	$(CC) $(LDFLAGS) -o $@ $(FRONTEND_OBJS) $(MP3LIB) $(LDLIBS)

$(MP3LIB): $(LIBMP3LAME_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBMP3LAME_OBJS)
	$(RANLIB) $@

%.o: %.c config.h Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: $(FRONTEND)
	$(FRONTEND) --nores testcase.wav testcase.new.mp3
	@echo
	@echo "The following output has value only for a LAME-developer, do not make _any_"
	@echo "assumptions about what this number means. You do not need to care about it."
	@{ git diff --numstat --no-index testcase.mp3 testcase.new.mp3 || true; } | awk 'NF { total += $$1 + $$2 } END { print total + 0 }'

install: $(MP3LIB)
	$(MKDIR_P) $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include/lame $(DESTDIR)$(PREFIX)/lib/pkgconfig
	$(INSTALL_DATA) $(MP3LIB) $(DESTDIR)$(PREFIX)/lib/libmp3lame.a
	$(INSTALL_DATA) include/lame.h $(DESTDIR)$(PREFIX)/include/lame/lame.h
	{ \
		echo "prefix=$(PREFIX)"; \
		echo 'exec_prefix=$${prefix}'; \
		echo 'libdir=$${exec_prefix}/lib'; \
		echo 'includedir=$${prefix}/include'; \
		echo; \
		echo "Name: mp3lame"; \
		echo "Description: MPEG Layer 3 audio codec"; \
		echo "Version: $(VERSION)"; \
		echo 'Libs: -L$${libdir} -lmp3lame'; \
		echo 'Cflags: -I$${includedir}'; \
	} > $(DESTDIR)$(PREFIX)/lib/pkgconfig/mp3lame.pc

clean:
	$(RM) $(FRONTEND) $(MP3LIB) $(OBJS) $(DEPS) testcase.new.mp3

-include $(DEPS)
