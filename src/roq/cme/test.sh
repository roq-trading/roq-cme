#!/usr/bin/env bash

NAME="cme"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-cme/$CONFIG.toml"

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
  --exchange "XCBT,XNYM" \
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

# $PREFIX "./roq-cme" \
#   --name=cme \
#   --event_log_symlink=true \
#   --multicast_channel_ids=344 \
#   --multicast_local_interface=172.19.0.1 \
#   --multicast_buffer_depth=2 \
#   --event_log_iso_week=false
#   --enable_market_by_order=true
#   --test_mbp_to_mbo_clear_price_level=true
#   --ilink_firm_id=ROQ
#   --ilink_port=65301
#   --ilink_market_segment_ids=84
#   --ilink_location=UK
#   --config_file=$HOME/etc/cme/config.toml \
#   --client_listen_address=$HOME/run/cme.sock \
#   --service_listen_address=$HOME/run/cme-service.sock \
#   --cache_dir=$HOME/var/lib/roq/cache \
#   --event_log_dir=$HOME/var/lib/roq/data \
#   --multicast_config_file=$SHARE/config.xml \
#   --ilink_config_file=$SHARE/MSGW_Config.xml \
#   --auth_keys_file=/home/quincy/keys.json \
#   --secdef_config_file=$HOME/secdef.dat \
#   $@
