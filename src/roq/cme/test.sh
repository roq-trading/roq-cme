#!/usr/bin/env bash

NAME="deribit"

CONFIG_FILE="config/$NAME.toml"
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
	--name "$NAME" \
	--config_file "$CONFIG_FILE" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink \
	--client_listen_address "$HOME/run/$NAME.sock" \
	--metrics_listen_address "$HOME/run/${NAME}_metrics.sock" \
	--multicast_channel_id "310" \
	--multicast_config_file "$MULTICAST_CONFIG_FILE" \
	--multicast_local_interface "1.2.3.4" \
	$@
