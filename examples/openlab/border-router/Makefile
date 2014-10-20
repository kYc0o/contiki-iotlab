all: border-router

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" -DWITH_UIP6

PROJECT_SOURCEFILES += slip-bridge.c


# For openlab nodes
#     Should run make clean after modifying CFLAGS
## CFLAGS += -DSLIP_ARCH_CONF_BAUDRATE=115200

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
