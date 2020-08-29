#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  gtag=${gtag}-dr
fi
docker container run \
  --volume=$(pwd)/image/scripts:/scripts \
  --volume=$1:/srcdir \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=buildpro_${gtag} \
  --name=buildpro_build \
  --rm -it buildpro:${gtag} \
  build
