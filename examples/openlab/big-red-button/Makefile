all: red-button

CFLAGS+= -DPROJECT_CONF_H=\"project-conf.h\" -DWITH_UIP6

PROJECT_SOURCEFILES += fox_gpio_button.c gpio_button.c
PROJECT_SOURCEFILES += command_parser.c console_echo.c
PROJECT_SOURCEFILES += web_service.c uip_util.c
PROJECT_SOURCEFILES += httpd-simple.c http-server.c


CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
