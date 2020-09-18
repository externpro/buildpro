#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=${gtag}-dr
fi
for img in centos6-bld
do
  time docker image build \
    --network=host \
    --build-arg USERNAME=${USER} \
    --build-arg USERID=$(id -u ${USER}) \
    --file ${img}.dockerfile \
    --tag buildpro/${img}:${gtag} .
done
docker image ls
