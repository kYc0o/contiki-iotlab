#! /bin/bash

cd $(dirname $0)

make -C ..

ROOT_IOTLAB="../../../../../"

PYTHONPATH="${ROOT_IOTLAB}/tools_and_scripts/:${ROOT_IOTLAB}/cli-tools/:."
export PYTHONPATH

nosetests contiki_sensors_collecting_test.py
