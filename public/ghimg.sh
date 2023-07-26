#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no)" ] || [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
fi
# build ghcr.io images
for img in rocky9-pro rocky9-bld
do
  pkg=ghcr.io/smanders/buildpro/${img}:${gtag}
  time docker image build \
    --network=host \
    --file ${img}.dockerfile \
    --tag ghcr.io/smanders/buildpro/${img}:latest \
    --tag ${pkg} .
  docker push ghcr.io/smanders/buildpro/${img}:${gtag}
done
docker image ls
