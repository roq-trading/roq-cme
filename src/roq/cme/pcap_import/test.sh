#!/usr/bin/env bash
./roq-cme-pcap-import \
  --config_file ../config/config.xml \
  --channel_ids 344 \
  --symbols 'ZN[HMUZ][0-9]' \
  $@
