#!/bin/bash


source ./nodes_conf.sh

node=($nodes)

STEP() { sep=${1//?/=}; printf "$sep\n%s\n$sep\n" "$1"; }

printf "we will need sudo: "; sudo echo ok

STEP "settings"
set -x
firmware=./sensors-collector.iotlab-m3
iotlab_site="grenoble"
border_router="m3-${node[0]}"
nodes=`tr -s ' ' '+' <<< "$nodes"`
set +x &> /dev/null

STEP "configuring global host ipv6 address (lo=$web_server_addr)"
sudo ifconfig lo add $web_server_addr/128 &>/dev/null

STEP "launching experiment"
exp_id=`experiment-cli submit -d 60 -l $iotlab_site,m3,$nodes,$firmware \
	| awk '/id/ {print $2}'`

STEP "proxyfying border-router's serial port"
ssh ${iotlab_site}.iot-lab.info -N -L 2000:$border_router:20000 &
trap "kill %1" EXIT

STEP "waiting for experiment to start"
while ! experiment-cli get --exp-state --id $exp_id \
	| egrep -q 'Running|Error'; do sleep 1; done

experiment-cli get --exp-state --id $exp_id | grep -q 'Error' \
&& echo "ERROR starting experiment" && exit 1

STEP "checking for deployment failures"
failed=`~/IoT-LAB/iot-lab/qualif/get_failed_nodes.sh $exp_id`
[ "$failed" ] && echo "$failed" || echo "all ok"

STEP "starting tunslip6 bridge"
sudo `which tunslip6` babe::1/64 -a localhost -p 2000 &>/dev/null &

STEP "waiting for exeriment to end"
while ! experiment-cli get --exp-state --id $exp_id \
	| egrep -q 'Terminated|Error'; do sleep 1; done

STEP "completed"
