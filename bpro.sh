#!/usr/bin/env bash
cd "$( dirname "$0" )"
CMD=shell
MOUNT=$HOME
REPO=buildpro/centos6-bld
TAG=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  TAG=${TAG}-dr
fi
CONTAINER_HOSTNAME=buildpro_${TAG}
while getopts ":c:m:r:t:" opt
do
  case ${opt} in
    c )
      CMD=$OPTARG
      ;;
    m )
      MOUNT=$OPTARG
      ;;
    r )
      REPO=$OPTARG
      ;;
    t )
      TAG=$OPTARG
      CONTAINER_HOSTNAME=buildpro_${TAG}
      ;;
    \? )
      echo "Invalid option: $OPTARG" 1>&2
      exit
      ;;
    : )
      echo "Invalid option: $OPTARG requires an argument" 1>&2
      exit
      ;;
  esac
done
shift $((OPTIND -1))
docker container run \
  --volume=$(pwd)/image/scripts:/scripts \
  --volume=$MOUNT:/srcdir \
  --volume="$HOME/.Xauthority:/root/.Xauthority:rw" \
  --volume="$HOME/.ssh:/home/${USER}/.ssh" \
  --env="DISPLAY" \
  --net=host \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=${CONTAINER_HOSTNAME} \
  --name=buildpro_${CMD} \
  --rm -it ${REPO}:${TAG} \
  ${CMD}
