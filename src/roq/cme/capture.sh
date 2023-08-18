#!/usr/bin/env bash

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

# channel 344

case "$ENV" in
  prod)
    FILTER=$(
    echo \
      "(port 14344 and (host 224.0.31.68 or host 224.0.31.110 or host 224.0.31.89)) or " \
      "(port 15344 and (host 224.0.32.68 or host 224.0.32.110 or host 224.0.32.89)) or " \
      "(port 23344 and host 233.72.75.33) or " \
      "(port 22344 and host 233.72.75.96)"
    )
    ;;
  test)
    FILTER=$(
    echo \
      "(port 14344 and host 233.158.8.17) or " \
      "(port 15344 and host 233.158.8.144) or " \
      "(port 6344 and (host 233.158.8.101 or host 233.158.8.59)) or " \
      "(port 7344 and (host 233.158.8.228 or host 233.158.8.186)) or " \
      "(port 21344 and host 224.0.25.191) or " \
      "(port 22344 and host 224.0.25.93)"
    )
    ;;
  cert)
    ;;
  *)
    (>&2 echo -e "\033[1;31mERROR: Invalid environment. Should be one of 'prod', 'test' or 'cert'.\033[0m") && exit 1
    ;;
esac


OUTPUT="test.pcap"

echo "FILTER=$FILTER"
echo "OUTPUT=$OUTPUT"

tcpdump -i $INTERFACE -w $OUTPUT $FILTER
