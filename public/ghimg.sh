#!/usr/bin/env bash
cd "$( dirname "$0" )"
# build local/test images
if [ "$#" -gt 0 ]; then
  images=("$@")
else
  images=(ubuntu rocky10-gcc15 rocky9-gcc13 rocky8-gcc9 rocky-mdv rocky-ci rocky-pin rocky-pdv)
fi
for img in "${images[@]}"
do
  time docker image build \
    --network=host \
    --build-arg BPROTAG=latest \
    --file ${img}.dockerfile \
    --tag buildpro/${img}:latest .
done
