ifndef TARGET
TARGET=iotlab-m3
endif

all: rpl-node-info

UIP_CONF_IPV6 = 1
CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\"

PROJECT_SOURCEFILES += ../ipv6_common.c


CONTIKI = ../../../..
include $(CONTIKI)/Makefile.include
