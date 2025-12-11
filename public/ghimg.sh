#!/usr/bin/env bash
cd "$( dirname "$0" )"
# build local/test images
for img in ubuntu rocky10-gcc15 rocky9-gcc13 rocky8-gcc9 rocky-mdv rocky-ci rocky-pin rocky-pdv
do
  time docker image build \
    --network=host \
    --build-arg BPROTAG=latest \
    --file ${img}.dockerfile \
    --tag buildpro/${img}:latest .
done
