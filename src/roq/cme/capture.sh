#!/usr/bin/env bash

# channel 344

FILTER=$(
echo \
  "(port 14344 and (host 224.0.31.68 or host 224.0.31.110 or host 224.0.31.89 )) or " \
  "(port 15344 and (host 224.0.32.68 or host 224.0.32.110 or host 224.0.32.89 )) or " \
  "(port 23344 and host 233.72.75.33) or " \
  "(port 22344 and host 233.72.75.96)"
)

echo "FILTER=$FILTER"

tcpdump \
  -i enp2s0 \
  -s 65535 \
  -w test.pcap \
  $FILTER
