#!/usr/bin/env bash

size_common=$1
size_not_common=$2
nb_alternates=$3

gen_common="cat /dev/urandom | base64 | tr -dc \"[:alnum:]\" | head -c${size_common} | tr -s \"\n\" \" \"|tr -d \" \""
gen_uncommon="cat /dev/urandom | base64 | tr -dc \"[:alnum:]\" | head -c${size_not_common} | tr -s \"\n\" \" \"|tr -d \" \""

echo "" > /tmp/a
echo "" > /tmp/b
for i in $(seq 0 $nb_alternates); do
  if [[ $((${i}%2)) == 0 ]]; then
    eval ${gen_common} > /tmp/common
    cat /tmp/common >> /tmp/a
    cat /tmp/common >> /tmp/b
  else
    eval ${gen_uncommon} >> /tmp/a
    eval ${gen_uncommon} >> /tmp/b
  fi
done



