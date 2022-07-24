#!/usr/bin/env bash

NAME="deribit"

CONFIG_FILE="config/$NAME.toml"

URI="deribit.com"

FIX_URI="tcp://www.$URI:9881"
WS_URI="wss://www.$URI/ws/api/v2"

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

$PREFIX "./roq-deribit" \
	--name "$NAME" \
	--config_file "$CONFIG_FILE" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink \
	--client_listen_address "$HOME/run/$NAME.sock" \
	--metrics_listen_address "$HOME/run/${NAME}_metrics.sock" \
	--fix_uri "$FIX_URI" \
	--ws_uri "$WS_URI" \
	$@
