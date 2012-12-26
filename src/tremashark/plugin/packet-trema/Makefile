WIRESHARK_SRC_DIR = ../wireshark

SRCS   = packet-trema.c plugin.c
CC     = gcc
OBJS   = $(foreach src, $(SRCS), $(src:.c=.o))

PLUGIN_NAME = packet-trema

# local installation path
LOCAL_PLUGIN_DIR  = /home/$(shell whoami)/.wireshark/plugins

# determine global installation path (use latest plugin version path)
ifneq ($(wildcard /usr/local/lib/wireshark/plugins),)
	GLOBAL_PLUGIN_DIR_PARTIAL=/usr/local/lib/wireshark/plugins
else ifneq ($(wildcard /usr/lib/wireshark/plugins),)
	GLOBAL_PLUGIN_DIR_PARTIAL=/usr/lib/wireshark/plugins
endif

# Work out if there are version-specific subdirectories
GLOBAL_PLUGIN_DIR_FILES := $(wildcard $(GLOBAL_PLUGIN_DIR_PARTIAL)/*)
GLOBAL_PLUGIN_DIR_SUBDIRS := $(foreach file,$(GLOBAL_PLUGIN_DIR_FILES),$(dir $(wildcard $(file)/.)))
GLOBAL_PLUGIN_DIR_SUBDIRS := $(subst $(GLOBAL_PLUGIN_DIR_PARTIAL)/,,$(GLOBAL_PLUGIN_DIR_SUBDIRS))
GLOBAL_PLUGIN_DIR_SUBDIRS := $(subst /,,$(GLOBAL_PLUGIN_DIR_SUBDIRS))

# Assume that the "last" directory is the one we ant to use if it exists
GLOBAL_PLUGIN_VER := $(lastword $(sort $(GLOBAL_PLUGIN_DIR_SUBDIRS)))

# Create the actual global plugin dir to use
ifneq ($(GLOBAL_PLUGIN_VER),)
	GLOBAL_PLUGIN_DIR=$(GLOBAL_PLUGIN_DIR_PARTIAL)/$(GLOBAL_PLUGIN_VER)
else
	GLOBAL_PLUGIN_DIR=$(GLOBAL_PLUGIN_DIR_PARTIAL)
endif

OSTYPE = $(shell uname)
ifeq ($(OSTYPE),Linux)
ENDIAN=-D_LITTLE_ENDIAN_
endif
ifeq ($(OSTYPE),SunOS)
ENDIAN=-D_BIG_ENDIAN_
endif

INC_GLIB = -I/usr/include/glib-2.0 -I/usr/local/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
INC_OPENFLOW = -I../../../../objects/openflow
INC_TREMA = -I../../../lib

INC_DIRS = -I. $(INC_GLIB) $(INC_OPENFLOW) $(INC_TREMA)
CFLAGS = -std=gnu99 -D_GNU_SOURCE $(INC_DIRS) -DHAVE_CONFIG_H -I$(WIRESHARK_SRC_DIR) -I/usr/local/include -I/usr/local/include -DINET6 -D_U_=__attribute__\(\(unused\)\) -Wall -Wpointer-arith -g -I/usr/local/include -DXTHREADS -D_REENTRANT -DXUSE_MTSAFE_API -pthread -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/X11R6/include -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/freetype2/config -fPIC -DPIC $(ENDIAN) $(DISSECT_PORT)

LDFLAGS = -Wl,--rpath -Wl,/usr/local/lib -Wl,--rpath -Wl,/usr/local/lib -L/usr/local/lib -L$(WIRESHARK_SRC_DIR)/epan -L. -lgmodule-2.0 -ldl -lglib-2.0 -pthread -Wl,--export-dynamic -Wl,-soname -Wl,$(PLUGIN_NAME).so

.PHONY: clean install

$(PLUGIN_NAME).so : $(OBJS) $(SRCS)
	$(CC) -shared $(OBJS) $(LDFLAGS) -o $@

install: $(PLUGIN_NAME).so
	@if [ `id -u` -eq 0 ]; then \
	  if [ -d "$(GLOBAL_PLUGIN_DIR)" ]; then \
	    target="$(GLOBAL_PLUGIN_DIR)/$<"; \
	    res="*** Installed plugin for ALL users ($$target)"; \
	  else \
	    echo "*** Error: global plugin directory $(GLOBAL_PLUGIN_DIR) does not exist"; \
	    exit 1; \
	  fi; \
	else \
	  mkdir -p "$(LOCAL_PLUGIN_DIR)/"; \
	  target="$(LOCAL_PLUGIN_DIR)/$<"; \
	  res="*** Installed plugin for user "`id | cut -d\( -f2 | cut -d\) -f1`" ($$target)"; \
	fi; \
	install --mode=644 $< "$$target" && echo "" && echo "" && echo "$$res" && echo "" && echo ""

plugin.c: moduleinfo.h Makefile.am Makefile.common
	$(MAKE) -f Makefile.am

clean:
	rm -f $(PLUGIN) $(OBJS) $(PLUGIN_NAME).so
