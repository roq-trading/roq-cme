#!/usr/bin/env bash

if [[ -z $CONDA_PREFIX ]]; then
  (>&2 echo -e "\033[1;31mERROR: Please activate your conda environment before using this script.\033[0m") && exit 1
fi

if [[ -z $1 ]]; then
    (>&2 echo -e "\033[1;31mERROR: Expected first argument to be interface name.\033[0m") && exit 1
fi

if [[ -z $2 ]]; then
  (>&2 echo -e "\033[1;31mERROR: Expected second argument to be environment. Use one of 'prod', 'test' or 'cert'.\033[0m") && exit 1
fi

INTERFACE="$2"
ENV="$1"

echo "INTERFACE=$INTERFACE"
echo "ENV=$ENV"

$CONFIG_FILE="$CONDA_PREFIX/share/roq-cme/$ENV/config.xml"

FILTER=$(roq-cme-pcap-filter --type tcpdump --config_file $CONFIG_FILE --channel_ids 344)

OUTPUT="test.pcap"

echo "FILTER=$FILTER"
echo "OUTPUT=$OUTPUT"

tcpdump -i $INTERFACE -w $OUTPUT $FILTER
