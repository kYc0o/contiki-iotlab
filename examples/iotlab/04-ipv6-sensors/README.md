IPv6 Sensors
============

Shows sensors information as a web-page available over http over IPv6,
and posts information over http to a configurable central location.


Overview
--------

This project contains a single firmware, to be flashed on all nodes.
The firmware integrates several functions:

* sensors poller
* http server
* http client
* border router

All nodes feature a simple http server, poll their sensors every other second,
and display collected information as json on their home page at ``/``.
In addition, network neighbourhood information is available at ``/network``.

All nodes post json sensor data over http to a central collector web-service,
if configured.  The destination of the post requests can be set individually
on each node using a get request: ``/set_destination?[<ipv6 address>]:port``.

Finally, when a node receives a post request, it stores the data for display
along with local sensors information; only the last received post is shown.


Networking, Border Router
-------------------------

Nodes are globally reachable via IPv6 through a RPL border router node, which
transfers RPL network IPv6 packets comming from or going to the external
network over it's serial link.  This is known as SLIP, for Serial Line IP.

The RPL border router must be bridged to the external network using tunslip6
on the host side: tunslip6 bridges IPv6 packets comming from the border router
over the serial link to the external network, and backwards too.  The pair
border-router + tunslip6 in effect implement a brouter - or bridge-router.

Finally, for all nodes to eventually be reachable and reach out to the
central collector web-service, radio connectivity must exist between nodes,
at least in clusters, and in chains to the border router node.


Nodes behaviour
---------------

When flashed with the firmware, nodes adopt one of two roles:

* sensor node
* border router

At startup, the firmware probes the node's serial link for tunslip6 presence
during 5 seconds by sending 5 prefix requests, one per second.  When tunslip6
is actually running on the other end of the serial link, the prefix response
triggers the firmware to build a new RPL dag and the node, from there on, acts
as a border-router, in addition to being a standard sensor node.


Limitations
-----------

The http server has a single shared receive buffer, so GET and POST requests
may interfere when received concurrently.  The POSTed data is limited to 1K,
due to limits on both the http server post-request buffer and on the local
storage buffer used to hold the last received data.

The http client uses the generator psock interface and places data directly
in the global uip buffer, which is sized to 1K in project-conf.h.  No check
is performed on bounds.

Build Configuration
-------------------

The firmware can be tuned at compile-time using parameters specified
in file ``project-conf.h``.  The default configuration is set to the following:
radio channel 22 (default is 11), tx power at 3dBm (default is 0dBm), and uIP
buffer size at 1K (default is 128B).  The uIP buffer size needs to be set big
enough to hold http get requests responses and http post requests contents.

