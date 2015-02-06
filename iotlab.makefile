tests: compile_tests
all: tests


compile_tests: compile_tests_iotlab-m3 compile_tests_iotlab-a8-m3 compile_tests_wsn430
compile_tests_%:
	# build supported contiki examples
	make -s -C examples/iotlab TARGET=$*

compile_tests_wsn430:
	# build some firmwares, not tested though for the moment
	# CC1100 Not working for the moment
	# make -s -C examples/hello-world TARGET=wsn430 WITH_CC1100=1 clean all
	make -s -C examples/hello-world TARGET=wsn430 WITH_CC2420=1 clean all

clean: clean_iotlab-m3 clean_iotlab-a8-m3 clean_wsn430
clean_%:
	make -s -C examples/iotlab TARGET=$* clean
clean_wsn430:
	make -s -C examples/hello-world TARGET=wsn430 clean

.PHONY: tests compile_tests clean
