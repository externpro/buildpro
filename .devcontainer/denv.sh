#!/usr/bin/env bash
cd "$( dirname "$0" )"
pushd .. > /dev/null
rel=$(grep FROM .devcontainer/centos7-bld.dockerfile | cut -d- -f2)
rel=${rel//:}
rel=bp${rel/./-}
display_host=$(echo ${DISPLAY} | cut -d: -f1)
if [[ -z "${display_host}" ]]; then
  display_env=${DISPLAY}
  xauth_env=
elif [[ "${display_host}" == "localhost" ]]; then
  echo "NOTE: X11UseLocalhost should be no in /etc/ssh/sshd_config"
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
##############################
if command -v host >/dev/null && host isrhub.usurf.usu.edu | grep "has address" >/dev/null; then
  urlPfx="https://isrhub.usurf.usu.edu"
else
  urlPfx=.
fi
# NOTE: EXTERN_DIR and GCC_VER need to match buildpro/image/centos7-pro.dockerfile
EXTERN_DIR=/opt/extern
GCC_VER=gcc731
##############################
if [ -f Nodejs/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" Nodejs/CMakeLists.txt`
elif [ -f WebClient/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" WebClient/CMakeLists.txt`
elif [ -f Web/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" Web/CMakeLists.txt`
elif [ -f web/CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" web/CMakeLists.txt`
elif [ -f CMakeLists.txt ]; then
  wpro=`grep "set(webpro_REV" CMakeLists.txt`
elif [ -f image/defaults.txt ]; then
  wpro=`grep "set(webpro_REV" image/defaults.txt`
fi
wproVer=`echo ${wpro} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
if [[ -n "${wproVer}" ]]; then
  wproBase=webpro-${wproVer}-${GCC_VER}-64-Linux
  if [[ ${wproVer} < "20.05.1" ]]; then
    WEBPRO="wget -q \"${urlPfx}/webpro/webpro/releases/download/${wproVer}/${wproBase}.sh\" \
&& chmod 755 webpro*.sh \
&& ./${wproBase}.sh --prefix=${EXTERN_DIR} --include-subdir \
&& rm ${wproBase}.sh"
  else
    WEBPRO="wget -qO- ${urlPfx}/webpro/webpro/releases/download/${wproVer}/${wproBase}.tar.xz | tar -xJ -C ${EXTERN_DIR}"
  fi
fi
env="${env}\nWEBPRO=${WEBPRO}"
##############################
if [ -f Shared/make/toplevel.cmake ]; then
  ipro=`grep "set(internpro_REV" Shared/make/toplevel.cmake`
elif [ -f SDKLibraries/make/toplevel.cmake ]; then
  ipro=`grep "set(internpro_REV" SDKLibraries/make/toplevel.cmake`
elif [ -f Libraries/cmake/toplevel.cmake ]; then
  ipro=`grep "set(internpro_REV" Libraries/cmake/toplevel.cmake`
elif [ -f CMakeLists.txt ]; then
  ipro=`grep "set(internpro_REV" CMakeLists.txt`
elif [ -f image/defaults.txt ]; then
  ipro=`grep "set(internpro_REV" image/defaults.txt`
fi
iproVer=`echo ${ipro} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
if [[ -n "${iproVer}" ]]; then
  INTERNPRO="wget -qO- ${urlPfx}/smanders/internpro/releases/download/${iproVer}/internpro-${iproVer}-${GCC_VER}-64-Linux.tar.xz | tar -xJ -C ${EXTERN_DIR}"
fi
env="${env}\nINTERNPRO=${INTERNPRO}"
##############################
if [ -f CMakeLists.txt ]; then
  psdk=`grep PluginSDK_REV CMakeLists.txt`
elif [ -f image/defaults.txt ]; then
  psdk=`grep PluginSDK_REV image/defaults.txt`
fi
psdkVer=`echo ${psdk} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d ")" -f1`
if [[ ${psdkVer} == "v3.0.3.0" ]]; then
  pfx=Vantage
else
  pfx=SDL
fi
if [[ -n "${psdkVer}" ]]; then
  PLUGINSDK="wget -qO- ${urlPfx}/PluginFramework/SDKSuper/releases/download/${psdkVer}/${pfx}PluginSDK-${psdkVer}-${GCC_VER}-64-Linux.tar.xz | tar -xJ -C ${EXTERN_DIR}"
fi
env="${env}\nPLUGINSDK=${PLUGINSDK}"
##############################
if [ -f .crtoolrc ]; then
  crtv=`grep version .crtoolrc`
elif [ -f image/defaults.txt ]; then
  crtv=`grep CRTOOL_REV image/defaults.txt`
fi
crToolVer=`echo ${crtv} | awk '{$1=$1};1' | cut -d " " -f2 | cut -d "\"" -f2`
crWrapVer=20.07.1
if [[ -n "${crToolVer}" && -n "${crWrapVer}" ]]; then
  CRTOOL="mkdir ${EXTERN_DIR}/CRTool \
&& wget -q \"${urlPfx}/CRTool/CRTool/releases/download/${crWrapVer}/CRTool-${crWrapVer}.sh\" \
&& wget -q \"${urlPfx}/CRTool/CRToolImpl/releases/download/${crToolVer}/CRToolImpl-${crToolVer}.sh\" \
&& chmod 755 CRTool*.sh \
&& ./CRTool-${crWrapVer}.sh --prefix=${EXTERN_DIR}/CRTool --exclude-subdir \
&& ./CRToolImpl-${crToolVer}.sh --prefix=${EXTERN_DIR} --include-subdir \
&& rm CRTool-${crWrapVer}.sh \
&& rm CRToolImpl-${crToolVer}.sh"
fi
env="${env}\nCRTOOL=${CRTOOL}"
##############################
echo -e "${env}" > .env
popd > /dev/null
