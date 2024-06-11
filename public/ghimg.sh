#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no)" ] || [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
fi
# build ghcr.io images
for img in rocky85-pro rocky85-bld rocky85-dev rocky85-run
do
  pkg=ghcr.io/externpro/buildpro/${img}:${gtag}
  time docker image build \
    --network=host \
    --build-arg BPROTAG=${gtag} \
    --file ${img}.dockerfile \
    --tag ghcr.io/externpro/buildpro/${img}:latest \
    --tag ${pkg} .
  docker push ghcr.io/externpro/buildpro/${img}:${gtag}
done
docker image ls
