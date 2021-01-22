#!/usr/bin/env bash
cd "$( dirname "$0" )"
pushd .. > /dev/null
rel=$(grep FROM .devcontainer/Dockerfile | cut -d- -f2)
rel=${rel//:}
rel=bp${rel/./-}
display_host=$(echo ${DISPLAY} | cut -d: -f1)
if [[ -z "${display_host}" ]]; then
  display_env=${DISPLAY}
  xauth_env=
elif [[ "${display_host}" == "localhost" ]]; then
  echo "NOTE: X11UseLocahost should be no in /etc/ssh/sshd_config"
else
  display_screen=$(echo $DISPLAY | cut -d: -f2)
  display_num=$(echo ${display_screen} | cut -d. -f1)
  magic_cookie=$(xauth list ${DISPLAY} | awk '{print $3}')
  xauth_file=/tmp/.X11-unix/docker.xauth
  docker_host=$(ip -4 addr show docker0 | grep -Po 'inet \K[\d.]+')
  touch ${xauth_file}
  xauth -f ${xauth_file} add ${docker_host}:${display_num} . ${magic_cookie}
  display_env=${docker_host}:${display_screen}
  xauth_env=${xauth_file}
fi
env="COMPOSE_PROJECT_NAME=${PWD##*/}"
env="${env}\nHNAME=${rel}"
env="${env}\nUSERID=$(id -u ${USER})"
env="${env}\nGROUPID=$(id -g ${USER})"
env="${env}\nDISPLAY_ENV=${display_env}"
env="${env}\nXAUTH_ENV=${xauth_env}"
if [ -f Nodejs/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" Nodejs/CMakeLists.txt`
elif [ -f WebClient/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" WebClient/CMakeLists.txt`
elif [ -f Web/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" Web/CMakeLists.txt`
elif [ -f web/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" web/CMakeLists.txt`
fi
WEBPRO=`echo ${wpro} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
if [ -f Shared/make/toplevel.cmake ]; then
  ipro=`grep "set(internpro_REV" Shared/make/toplevel.cmake`
elif [ -f SDKLibraries/make/toplevel.cmake ]; then
  ipro=`grep "set(internpro_REV" SDKLibraries/make/toplevel.cmake`
else
  ipro=`grep internpro_REV CMakeLists.txt`
fi
INTERNPRO=`echo ${ipro} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
PLUGINSDK=`grep SDK_REV CMakeLists.txt | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
CRTOOL=`grep version .crtoolrc | awk '{$1=$1};1' | cut -d " " -f2 | cut -d "\"" -f2`
CRWRAP=20.07.1
[[ -n "${WEBPRO}" ]] && env="${env}\nWEBPRO=${WEBPRO}"
[[ -n "${INTERNPRO}" ]] && env="${env}\nINTERNPRO=${INTERNPRO}"
[[ -n "${PLUGINSDK}" ]] && env="${env}\nPLUGINSDK=${PLUGINSDK}"
[[ -n "${CRTOOL}" ]] && env="${env}\nCRTOOL=${CRTOOL}"
[[ -n "${CRWRAP}" ]] && env="${env}\nCRWRAP=${CRWRAP}"
echo -e "${env}" > .env
popd > /dev/null
