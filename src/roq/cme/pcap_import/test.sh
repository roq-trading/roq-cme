#!/usr/bin/env bash

SYMBOLS="^Z[TFNB][FGHJKMNQUVXZ][0-9]$"

./roq-cme-pcap-import \
  --name cme \
  --config_file ../config/config.xml \
  --channel_ids 344 \
  --symbols $SYMBOLS \
  --output_file test.roq \
  $@
