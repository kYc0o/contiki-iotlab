#!/bin/bash

set_1="4 5 11 13 14 17 18 21 23 25 29 30 31 32"
set_2="33 37 38 39 40 44 45 48 49 51 55 59 61 63 65 68 69"
set_3="359 360    363 366 367 368    371 374 375    376 377 379 380"
#nodes="$set_1 $set_2"
#nodes="$set_3"
#nodes="$set_2 $set_3 $set_1"

set_4="94 91 88 85 80 77 73 71 289 4 5 11 14 21 25 30 35 40 45 51 55 61 65 69"
set_5="175 167 155 151 146 140 136 130 125 118 114 109 105 101" 
set_6="179 185 189 194 198 203 290 297 303 309 314 318 324 328 335 339 344 349 354"
set_7="204 208 212 216 220 226 231 235 239 244 248 253 257 260 264 267 271 273 278 282 285"
#nodes="$set_4 $set_5 $set_6"
#nodes="$set_4 $set_7 $set_6"

set_8="77 73 71 278 272 285 288 289 4 5 11"
nodes="$set_8"

# dead nodes: 2 9 15 35 113

# /etc/hosts: https://github.com/iot-lab/iot-lab/wiki/M3-GRE-iotlab-ipv6
node_lladdr() { grep -F "m3-$1." /etc/hosts | sed 's/.*:://; s/ .*//'; }
ipv6_prefix="BABE"

web_server_addr="beef::1"
web_server_port="8080"
