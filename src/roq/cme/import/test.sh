#!/usr/bin/env bash

CONFIG_FILE="../config/config.xml"
#SYMBOLS="^Z[TFNB][FGHJKMNQUVXZ][0-9]$"
SYMBOLS="ZNU3"

./roq-cme-import \
  --type pcap \
  --config_file $CONFIG_FILE \
  --channel_ids 344 \
  --name cme \
  --symbols $SYMBOLS \
  --output_file test.roq \
  $@
