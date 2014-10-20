tests: compile_tests
all: tests


compile_tests: compile_tests_iotlab-m3 compile_tests_iotlab-a8-m3
compile_tests_%:
	# build supported contiki examples
	make -s -C examples/iotlab TARGET=$*

clean: clean_iotlab-m3 clean_iotlab-a8-m3
clean_%:
	make -s -C examples/iotlab TARGET=$* clean

.PHONY: tests compile_tests clean
