ifndef TARGET
TARGET=iotlab-m3
endif

RIME_FOLDER = ../../rime
RIME_EXAMPLES = example-abc example-mesh example-collect example-trickle \
				example-polite example-rucb example-runicast example-unicast \
				example-neighbors
# Need cfs coffe:
#               example-rudolph0 example-rudolph1 example-rudolph2

TARGET_EXAMPLES = $(addsuffix .$(TARGET), $(RIME_EXAMPLES))


all: $(TARGET_EXAMPLES)

%.${TARGET}: ${RIME_FOLDER}/%.${TARGET}
	cp $^ $@

${RIME_FOLDER}/%:
	make -C ${RIME_FOLDER} TARGET=${TARGET} $*


clean:
	rm -f $(TARGET_EXAMPLES)
	rm -f $(addprefix $(RIME_FOLDER)/, $(TARGET_EXAMPLES))
	make -C ${RIME_FOLDER} TARGET=${TARGET} clean

