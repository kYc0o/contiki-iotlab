ifndef TARGET
TARGET = iotlab-m3
endif

APPLIS = rpl-dag-root rpl-node-info rpl-border-router

all: $(addprefix build-, $(APPLIS))
clean: $(addprefix clean-,$(APPLIS))

build-%:
	make -C $* TARGET=$(TARGET)

clean-%:
	make -C $* TARGET=$(TARGET) clean
	rm -f $*/$*.$(TARGET)

.PHONY: all clean clean-% build-%
