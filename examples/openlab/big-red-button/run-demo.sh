#/bin/bash

#set -x
set -e

br_device_tty=/dev/ttyUSB1
button_lladdr=323:4500:3236:3777 # fox36

baud_rate=115200
#baud_rate=500000

assert() { eval "$1" &> /dev/null || { echo $2; exit 1; } }
STEP() { printf "%-50s" "$1"; }

STEP "checking tunslip6 & lo"
tunslip6=$(dirname $0)/../../../tools/tunslip6
assert "test -x $tunslip" "$tunslip not found"
assert "ifconfig lo | grep RUNNING" "lo not running"
echo ok.

STEP "connect the border-router, then press any key"
read -n 1
assert "ls $br_device_tty" "border-router: $br_device_tty not found"

STEP "checking sudo right"
sudo echo ok.

STEP "starting tunslip6 serial/ipv6 bridge"
wpan_prefix=babe::
(cd ../../../tools
 sudo ./tunslip6 $wpan_prefix"1/64" -s $br_device_tty -B $baud_rate
) &> /tmp/tunslip6.log &
echo ok.

STEP "waiting for red-button http access"
while ! curl -sg "http://[$wpan_prefix$button_lladdr]/" > /dev/null ; do
	sleep 1
done
echo ok.

STEP "configuring host lo global ipv6 address"
host_addr=b00b::2
sudo ifconfig lo del $host_addr/128 &> /dev/null || true
sudo ifconfig lo add $host_addr/128
echo ok.

STEP "configuring red-button destination"
curl -sg "http://[$wpan_prefix$button_lladdr]/set_destination?$host_addr"\
| grep -q dest_address=
echo ok.

STEP "starting notification web-server"
(while true; do
	nc -6 -l 8000 | grep button_state || exit 1
done
) &
ws_pid=$!
disown %%
echo ok.

echo
echo "now, hit the red button !"
echo
echo "[press any key to terminate demo]"
echo
read -n 1

STEP "de-configuring host ipv6 & stopping processes"
sudo ifconfig lo del $host_addr/128
sudo killall tunslip6
kill $ws_pid
killall nc
echo ok.
