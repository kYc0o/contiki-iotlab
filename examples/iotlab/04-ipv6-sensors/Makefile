all: sensors-collector

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" -DWITH_UIP6

PROJECT_SOURCEFILES += border-router.c sensors-poller.c
PROJECT_SOURCEFILES += http-client.c
PROJECT_SOURCEFILES += httpd-simple.c http-server.c
PROJECT_SOURCEFILES += uip_util.c


CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
