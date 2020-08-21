#!/usr/bin/env bash
cd "$( dirname "$0" )"
time docker image build \
  --network=host \
  --build-arg USERNAME=${USER} \
  --build-arg USERID=$(id -u ${USER}) \
  --file ubuntu_18.04.dockerfile \
  --tag buildpro:v1 .
docker image ls
