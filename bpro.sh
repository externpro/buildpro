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
XARG="--env=DISPLAY=${DISPLAY}"
while getopts ":c:m:r:t:x" opt
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
    x )
      DOCKER_HOST=$(ip -4 addr show docker0 | grep -Po 'inet \K[\d.]+')
      DISPLAY_SCREEN=$(echo $DISPLAY | cut -d: -f2)
      DISPLAY_NUM=$(echo ${DISPLAY_SCREEN} | cut -d. -f1)
      MAGIC_COOKIE=$(xauth list ${DISPLAY} | awk '{print $3}')
      XAUTH=/tmp/.docker.xauth
      touch ${XAUTH}
      xauth -f ${XAUTH} add ${DOCKER_HOST}:${DISPLAY_NUM} . ${MAGIC_COOKIE}
      XARG="--env=DISPLAY=${DOCKER_HOST}:${DISPLAY_SCREEN}
        --volume=${XAUTH}:${XAUTH} --env=XAUTHORITY=${XAUTH}"
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
  --volume="$HOME/.ssh:/home/${USER}/.ssh" \
  --volume=/tmp/.X11-unix:/tmp/.X11-unix \
  ${XARG} \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=${CONTAINER_HOSTNAME} \
  --rm -it ${REPO}:${TAG} \
  ${CMD}
