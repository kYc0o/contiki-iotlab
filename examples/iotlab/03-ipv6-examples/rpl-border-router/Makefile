all: rpl-border-router

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" -DWITH_UIP6

PROJECT_SOURCEFILES += slip-bridge.c

CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
