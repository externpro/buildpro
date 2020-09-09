#!/usr/bin/env bash
cd "$( dirname "$0" )"
GTAG=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  GTAG=${GTAG}-dr
fi
MOUNT=$HOME
CMD=shell
if [ -d "$1" ]; then
  MOUNT=$1
  if [ -n "$2" ]; then
    CMD=$2
  fi
elif [ -n "$1" ]; then
  CMD=$1
fi
docker container run \
  --volume=$(pwd)/image/scripts:/scripts \
  --volume=$MOUNT:/srcdir \
  --volume="$HOME/.Xauthority:/root/.Xauthority:rw" \
  --volume="$HOME/.ssh:/home/${USER}/.ssh" \
  --env="DISPLAY" \
  --net=host \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=buildpro_${GTAG} \
  --name=buildpro_${CMD} \
  --rm -it buildpro:${GTAG} \
  ${CMD}
