#!/usr/bin/env bash

INTERFACE="10.247.43.74"  # eth1@if11

OUTPUT="test.pcap"

# channel 344

FILTER=$(
echo \
  "(port 14344 and (host 224.0.31.68 or host 224.0.31.110 or host 224.0.31.89 )) or " \
  "(port 15344 and (host 224.0.32.68 or host 224.0.32.110 or host 224.0.32.89 )) or " \
  "(port 23344 and host 233.72.75.33) or " \
  "(port 22344 and host 233.72.75.96)"
)

echo "INTEFACE=$INTERFACE"
echo "FILTER=$FILTER"
echo "OUTPUT=$OUTPUT"

tcpdump -i $INTERFACE -w $OUTPUT $FILTER
