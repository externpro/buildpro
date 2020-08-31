#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=${gtag}-dr
fi
mount=$HOME
cmd=shell
if [ -d "$1" ]; then
  mount=$1
  if [ -n "$2" ]; then
    cmd=$2
  fi
elif [ -n "$1" ]; then
  cmd=$1
fi
docker container run \
  --volume=$(pwd)/image/scripts:/scripts \
  --volume=$mount:/srcdir \
  --volume="$HOME/.Xauthority:/root/.Xauthority:rw" \
  --volume="$HOME/.ssh:/home/${USER}/.ssh" \
  --env="DISPLAY" \
  --net=host \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=buildpro_${gtag} \
  --name=buildpro_${cmd} \
  --rm -it buildpro:${gtag} \
  ${cmd}
