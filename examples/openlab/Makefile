ifndef TARGET
TARGET=openlab-fox
endif

all: node

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" -DWITH_UIP6


CONTIKI = ../..
include $(CONTIKI)/Makefile.include
