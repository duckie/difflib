#!/usr/bin/env bash

size_common=$1
size_not_common=$2
nb_alternates=$3

function gen_random {
  cat /dev/urandom | base64 | tr -dc "[:alnum:]" | head -c$1 | tr -s "\n" " "|tr -d " "
}

echo "" > /tmp/a
echo "" > /tmp/b
for i in $(seq 0 $nb_alternates); do
  if [[ $((${i}%2)) == 0 ]]; then
    gen_random $(($RANDOM%${size_common})) > /tmp/common
    cat /tmp/common >> /tmp/a
    cat /tmp/common >> /tmp/b
  else
    gen_random $(($RANDOM%${size_not_common})) >> /tmp/a
    gen_random $(($RANDOM%${size_not_common})) >> /tmp/b
  fi
done



