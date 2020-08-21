#!/usr/bin/env bash
cd /srcdir
if [ $1 = "shell" ]; then
  echo "starting bash shell"
  /bin/bash
elif [ $1 = "build" ]; then
  echo "performing software build"
  gcc helloworld.c -o helloworld -Wall
fi
