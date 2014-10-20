Border Router
=============

This firmware provides a simple border router for a RPL network,
using a slip-bridge to send/receive ipv6 packets over serial link.

The firmware sends prefix requests over the serial line to a
supposedly running tunslip6 on the other end of the line.
When received, the prefix serves to create a new RPL DAG.

The device's red led remains on until the prefix is received,
with occasional flashes depending on radio traffic.  After that,
leds flash only according to radio taffic.

To start the tunslip6 process on the host, use the following:
```
  $ contiki/tools/tunslip6 baba::1/64 -t tun0 -s /dev/ttyUSB1
```

Note 1: to build tunslip6: ``cd contiki/tools && make tunslip6``

Note 2: if using an iotlab-m3 device connected directly to the
usb, add option ``-B 500000`` to command ``tunslip6`` above.
