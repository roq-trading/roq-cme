#!/usr/bin/env bash

NAME="cme"

CONFIG_FILE="config/$NAME.toml"
SECDEF_CONFIG_FILE="config/secdef.dat"
MULTICAST_CONFIG_FILE="config/config.xml"
ILINK_CONFIG_FILE="../../../share/test/MSGW_Config.xml"
SECDEF_CONFIG_FILE="$HOME/secdef.dat"

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
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --multicast_channel_ids "310" \
  --multicast_config_file "$MULTICAST_CONFIG_FILE" \
  --multicast_local_interface "1.2.3.4" \
  --ilink_firm_id "ROQ" \
  --ilink_market_segment_ids "84" \
  --ilink_config_file="$ILINK_CONFIG_FILE" \
  --secdef_config_file="$SECDEF_CONFIG_FILE" \
  $@
