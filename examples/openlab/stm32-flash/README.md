STM32-FLASH
===========

Uses the on-chip "firmware flash" as pseudo-permanent storage,
and shows basic SMT32 memory mapping.

* basic STM32 memory mapping: .data and .bss segments in RAM
* on-chip flash memory: sections at the bottom of the addressing space
* writing to the on-chip flash memory: use openlab on-chip flash API
 
To see it in action: build firmware, create experiment, reset node.

* ``make`` (build firmware)
* ``experiment-cli submit -d 10 -l grenoble,m3,66,./stm32-flash.iotlab-m3``
* ``ssh grenoble.iot-lab.info nc m3-66 20000``  (check serial line output)
* ``node-cli --reset`` (reboot node)
* ``node-cli --update ./stm32-flash.iotlab-m3`` (re-flash firmware)

OpenOCD erases only the required number of pages starting from the bottom
of the memory when flashing a new firmware, so the last byte is safe
as long as the last page (2K) is not used to fit the firmware binary.


Hikob/Openlab on-chip Flash API:
--------------------------------

Hikob/Openlab provides a simple API to erase/write the on-chip flash:

* ``flash_erase_memory_page(page_address)``
* ``flash_write_memory_half_word(address, value)``

Erasing is per page (2K), and required before writing; addresses must
be aligned on page boundaries.  Writing is per half-word (16 bits).

Flash memory (512K) is mapped at addresses ``0x08000000 - 0x0807FFFF``
and aliased at ``0x00000 - 0x7FFFF``.


ST on-chip Flash APIs:
----------------------

ST provides two levels of API for accessing the STM32 on-chip flash:

* nvm.h: exposes one page of flash via readFrom/writeTo
* flash.h: low-level write access to flash pages via erase/write

The high-level API takes care of low-level flash accesses and alternates
writes to two separate pages to reduce wear-out.  This API is used for
radio calibration, which requires and reserves the lower 64 bytes of nvm. 

The low-level API offers per page erase and write primitives, along with
specific customer-information-block (cib) access.  Flash memory is mapped
at addresses ``0x08000000 - 0x0807FFFF`` and aliased at ``0x00000 - 0x7FFFF``.

Documentation for both ST APIs is available as part of the Contiki Doxygen
pages under Files > cpu > stm32w108 > hal > micro > cortexm3 > {nvm, flash}.h

Getting the ST API to compile for iotlab Contiki will require investigations.
