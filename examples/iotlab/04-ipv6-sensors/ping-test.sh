#!/bin/bash

source ./nodes_conf.sh

run_pinger() {
	nodes=${1:-$nodes}
	nodes=${nodes//+/ }
	nb_nodes=`echo $nodes | tr -c ' ' '\n' | wc -l`
	for id in $nodes; do
		lladdr=`node_lladdr "$id"`
		ping_process $id $ipv6_prefix::$lladdr $nb_nodes & 
	done
	trap '[ "`jobs`" ] && kill -ABRT `jobs -p`; /bin/rm /tmp/$$.*' EXIT
}

dump_status() {
	pinger_pid=$1
	status=`cat /tmp/$pinger_pid.*.state`
	nb_down=`tr -c -d ')' <<< "$status" | wc -c`
	nb_warm=`tr -c -d '|' <<< "$status" | wc -c`
	nb_total=`wc -l <<< "$status"`
	nb_up=$((nb_total-nb_down))
	echo $status
	echo
	printf "up: %d, down: %d, total: %d, warm: %d\n" \
		$nb_up $nb_down $nb_total $nb_warm
}

ping_process() {
	node_id=$1
	node_ipv6=$2
	nb_nodes=$3

	state_file="/tmp/$$.`printf "%03d" $node_id`.state"

	prev_state=
	while true; do
		state=`ping6 -c 1 -w 3 -s 1 $node_ipv6 &>/dev/null \
			&& printf "\e[32m%d\e[39m" $node_id \
			|| printf "\e[31m(%d)\e[39m" $node_id`
		stamp=`date +%T`
		if [ "$state" != "$prev_state" ]; then
			printf "%s %s\n" $stamp $state
		fi
		sleep_time=$((RANDOM/3276 * $nb_nodes / 5 + 10))
		for i in $(seq $sleep_time); do
			rem=$((sleep_time - i))
			if [ $rem -gt 3 ]; then
				flag="_"
			else
				flag="|"
			fi
			echo $state$flag > $state_file
			sleep 1
		done
		prev_state=$state
	done
}

gui_loop() {
	pid=$1
	while true; do
		$0 --status $pid \
		| awk '
			BEGIN { printf("\x1B[1;1H") } # goto screen 1,1
			END   { printf("\x1B[J")    } # erase end of screen
			{ printf("%s\x1B[K\n", $0)  } # erase end of line
		'
		sleep 1
	done
}

usage() {
	echo "usage: $0 [--run [<node ids>] | --status <pid> | --help]"
}

case $1 in
	--gui)
		gui_loop $2
		;;
	--status)
		dump_status $2
		;;
	--run|"")
		run_pinger "$2"
		xterm -geometry 110x5-0+0 -e "$0 --gui $$"
		;;
	--help|-h)
		usage
		;;
	*)
		echo "$0: unknown option '$1'" >&2 && exit 1
		;;
esac
