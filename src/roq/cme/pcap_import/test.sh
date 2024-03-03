#!/usr/bin/env bash

#  --symbols 'ZN[HMUZ][0-9]' \

./roq-cme-pcap-import \
  --name cme \
  --config_file ../config/config.xml \
  --channel_ids 344 \
  --symbols 'ZNU3' \
  --output_file test.roq \
  $@
