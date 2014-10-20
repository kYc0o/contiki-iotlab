Big Red Button Demo
===================


What You Need
-------------

- one pre-built Big Red Button device (Fox inside)
- one extra Fox device (to act as 'border router')
- one Fox ftdi-dongle + mini-usb
- one PC with linux and sudo access
- firmwares or build env

Setup the demo
--------------

- open-up the Big Red Button, access the Fox in it
- plug the ftdi-dongle on top of the extension board
- flash the red-button firmware
- start mini{com|term} and connect to the red-button cli
- type 'p' to print button state
- hit on the big red button
- type 'p' again and check state changed
- type 'l' to get the device ip address
- make a note of the button-ipv6, leaving out the fe80::
- exit minicom
- unplug the ftdi-dongle, place Fox back in Big Red Button
- plug the ftdi-dongle onto the border-router Fox
- flash the border-router firmware


Run the demo
------------

- start tunslip6 on relevant port for the border-router to use
```
    $ sudo ./tunslip6 babe::1/64 -s /dev/ttyUSB1 &
```
- configure the loopback network interface with a public ipv6 address
```
    $ sudo ifconfig lo add b00b::2/128
```
- configure the Big Red Button destination address to that of the host
```
    $ curl -gs 'http://[babe::<button-ipv6>]/set_destination?b00b::2'
```
- start a basic notification server on port 8000
```
    $ while true; do nc -6 -l 8000 | grep button_state; done
```
- now, hit the big red button !

- check the output of notification server to see the message


Notes
-----

- red-button: switch off the fox inside when demo done,
  back on again when starting anew, and refill battery at times
- border-router: use the firmware in ../border-router,
  if not, make sure uip stacks match, i.e. channel=22, buffer-size=1k
- if using spare iotlab-m3 node as BR, add tunslip6 option ``-B 500000``


Todo:
-----

- implement configuration of destination port
- implement battery-check using fox-button and led-flash code
- implement persistent destination address in micro-sd
- implement hardcoded button link-local ipv6 to ease demo setup


Building the Big Red Button device
----------------------------------

You need:
- one agile-fox
- one fox daughter board
- one big red button
- one resistor
- wires, solder

Recipe:
- wire the big red button to the daughter board
- plug the daughter board onto the Fox

