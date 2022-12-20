#!/usr/bin/env bash

NAME="cme"

CONFIG_FILE="config/$NAME.toml"
SECDEF_CONFIG_FILE="config/secdef.dat"
MULTICAST_CONFIG_FILE="config/config.xml"

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

$PREFIX "./roq-cme" \
  --name "cme" \
  --config_file "config/cme.toml" \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --multicast_channel_ids "310" \
  --multicast_config_file "config/config.xml" \
  --multicast_local_interface "1.2.3.4" \
  $@

#  --secdef_config_file "$SECDEF_CONFIG_FILE" \
