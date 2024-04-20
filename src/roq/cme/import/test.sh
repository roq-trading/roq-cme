#!/usr/bin/env bash

CONFIG_FILE="../config/config.xml"
#SYMBOLS="^Z[TFNB][FGHJKMNQUVXZ][0-9]$"
SYMBOLS="ZNU4"

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
  --type pcap \
  --config_file $CONFIG_FILE \
  --channel_ids 344 \
  --name cme \
  --symbols $SYMBOLS \
  --output_file test.roq \
  --secdef_config_file $HOME/secdef.dat \
  $@
