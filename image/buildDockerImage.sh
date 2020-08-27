#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=${gtag}-dr
fi
time docker image build \
  --network=host \
  --build-arg USERNAME=${USER} \
  --build-arg USERID=$(id -u ${USER}) \
  --file centos6.dockerfile \
  --tag buildpro:${gtag} .
docker image ls
