#!/usr/bin/env bash

CONFIG_FILE="../config/config.xml"
#SYMBOLS="^Z[TFNB][FGHJKMNQUVXZ][0-9]$"
SYMBOLS="ZNM4"

# debug?

if [ "$1" == "debug" ]; then
  KERNEL="$(uname -a)"
  case "$KERNEL" in
    Linux*)
      PREFIX="gdb --args"
      ;;
    Darwin*)
      PREFIX="lldb --"
      ;;
  esac
  shift 1
else
  PREFIX=
fi

# launch

$PREFIX ./roq-cme-import \
  --name cme \
  --type pcap \
  --channel_ids 344 \
  --symbols $SYMBOLS \
  --cme_config_file $CONFIG_FILE \
  --cme_secdef_file $HOME/secdef.dat \
  --event_log_output_file test.roq \
  $@
