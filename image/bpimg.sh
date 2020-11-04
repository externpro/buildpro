#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
elif [ -n "$(git status --porcelain --untracked=no)" ]; then
  gtag=working
fi
dodashu=true
host isrhub.usurf.usu.edu | grep "not found" >/dev/null && dodashu=false
for img in centos6-bld centos7-run centos7-bld
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
  if ${dodashu}; then
    time docker image build \
      --network=host \
      --build-arg USERNAME=${USER} \
      --build-arg USERID=$(id -u ${USER}) \
      --file ${img}-u.dockerfile \
      --tag bpro/${img}:${gtag} .
  else
    echo "isrhub.usurf.usu.edu not accessible"
  fi
done
docker image ls
