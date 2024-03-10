#!/usr/bin/env bash

CONFIG_FILE="../config/config.xml"

./roq-cme-filter \
  --type tcpdump \
  --config_file $CONFIG_FILE \
  --channel_ids 344
