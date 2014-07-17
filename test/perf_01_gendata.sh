#!/usr/bin/env bash

size=100
command="cat /dev/urandom | base64 | tr -dc \"[:alnum:]\" | head -c${size} | tr -s \"\n\" \" \"|tr -d \" \""

eval ${command} > /tmp/common

eval ${command} > /tmp/a1
eval ${command} > /tmp/a2

eval ${command} > /tmp/b1
eval ${command} > /tmp/b2

cat /tmp/a1 /tmp/common /tmp/a2 > /tmp/a
cat /tmp/b1 /tmp/common /tmp/b2 > /tmp/b


