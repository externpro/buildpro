#!/usr/bin/env bash
cd "$( dirname "$0" )"
# build local/test images
for img in ubuntu rocky-pro rocky-mdv rocky-ci rocky-pin rocky-pdv
do
  time docker image build \
    --network=host \
    --build-arg BPROTAG=latest \
    --file ${img}.dockerfile \
    --tag buildpro/${img}:latest .
done
