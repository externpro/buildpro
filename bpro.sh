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
XARG="--volume=/tmp/.X11-unix:/tmp/.X11-unix --env=DISPLAY=${DISPLAY}"
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
      CONTAINER_DISPLAY="0"
      DISPLAY_NUM=$(echo $DISPLAY | cut -d. -f1 | cut -d: -f2)
      AUTH_COOKIE=$(xauth list | grep "^$(hostname)/unix:${DISPLAY_NUM} " | awk '{print $3}')
      rm -rf .display
      mkdir -p .display/socket
      touch .display/Xauthority
      xauth -f .display/Xauthority add ${CONTAINER_HOSTNAME}/unix:${CONTAINER_DISPLAY} \
        MIT-MAGIC-COOKIE-1 ${AUTH_COOKIE}
      socat TCP4:localhost:60${DISPLAY_NUM} UNIX-LISTEN:.display/socket/X${CONTAINER_DISPLAY} &
      XARG="--volume=$(pwd)/.display/socket:/tmp/.X11-unix \
        --volume=$(pwd)/.display/Xauthority:/home/${USER}/.Xauthority \
        --env=DISPLAY=:${CONTAINER_DISPLAY}" # 172.17.0.1
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
  ${XARG} \
  --user=$(id -u ${USER}):$(id -g ${USER}) \
  --hostname=${CONTAINER_HOSTNAME} \
  --rm -it ${REPO}:${TAG} \
  ${CMD}
