#!/usr/bin/env bash
cd /bpvol
if [[ -z $1 ]]; then
  /bin/bash
elif [ $1 = "shell" ]; then
  /bin/bash
elif [ $1 = "build" ]; then
  echo "performing software build"
  gcc helloworld.c -o helloworld -Wall
fi
