#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=working
fi
for img in centos6-bld centos7-run
do
  pkg=ghcr.io/smanders/buildpro/${img}:${gtag}
  docker pull ${pkg}
  if [[ "$(docker images -q ${pkg} 2>/dev/null)" == "" || "${gtag}" == "working" ]]
  then
    time docker image build \
      --network=host \
      --file ${img}.dockerfile \
      --tag ghcr.io/smanders/buildpro/${img}:latest \
      --tag ${pkg} .
  fi
  time docker image build \
    --network=host \
    --build-arg USERNAME=${USER} \
    --build-arg USERID=$(id -u ${USER}) \
    --file ${img}-u.dockerfile \
    --tag bpro/${img}:${gtag} .
done
docker image ls
