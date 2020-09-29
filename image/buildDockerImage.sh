#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=working
fi
for img in centos6-bld centos7-run centos7-bld
do
  pkg=ghcr.io/smanders/buildpro/${img}:${gtag}
  docker pull ${pkg}
  if [[ "$(docker images -q ${pkg} 2>/dev/null)" == "" || "${gtag}" == "working" ]]
  then
    time docker image build \
      --network=host \
      --file ${img}.dockerfile \
      --tag ${pkg} .
  fi
  imgu=${img}-u
  time docker image build \
    --network=host \
    --build-arg USERNAME=${USER} \
    --build-arg USERID=$(id -u ${USER}) \
    --file ${imgu}.dockerfile \
    --tag buildpro/${imgu}:${gtag} .
done
docker image ls
