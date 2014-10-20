IPv6 Examples
=============

Examples using IPv6 and RPL.

RPL DAG root node
-----------------

### rpl-dag-root ###

Firmware that creates a RPL DAG. It creates a rpl dag with a hardcoded prefix.
It runs without tunslip6, and so, has no default route through the serial link.

### rpl-border-router ###

A regular contiki border router creating a rpl tree using prefix received from
tunslip6.


Nodes
-----

### rpl-node-info ###

A rpl node that prints informations about its network. IP addresses, routes,
neighbors.
