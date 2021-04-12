#!/usr/bin/env bash
cd "$( dirname "$0" )"
isrhubver=(\
  "WEBPRO=21.02"\
  "INTERNPRO=21.03"\
  "PLUGINSDK=v3.4.0.0"\
  "CRTOOL=20.10.1"\
  "CRWRAP=20.07.1"\
  )
for p in ${isrhubver[@]}; do
  bldargs+=("--build-arg $p")
done
args=
gtag=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no)" ] || [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
fi
doisrhub=false
command -v host >/dev/null \
  && host isrhub.usurf.usu.edu | grep "has address" >/dev/null \
  && doisrhub=true
# download/pull images (ghimg.sh builds them)
for img in centos7-run centos7-pro centos7-bld centos7-dev
do
  pkg=ghcr.io/smanders/buildpro/${img}:${gtag}
  docker pull ${pkg}
  if [[ "$(docker images -q ${pkg} 2>/dev/null)" == "" ]]; then
    echo "ghcr.io/smanders/buildpro/${img}:${gtag} not found: run ghimg.sh"
  fi
  dfile=../compose/.devcontainer/${img}.dockerfile
  awk -v r="${img}" -v t="${gtag}" '{gsub(/%BP_REPO%/,r);gsub(/%BP_TAG%/,t)} 1' bit.head.dockerfile > ${dfile}
  if ${doisrhub} && [[ ${img} == *"-bld"* ]]; then
    cat bit.isrhub.dockerfile >> ${dfile}
    cat bit.user.dockerfile >> ${dfile}
    args=${bldargs[@]}
  elif [[ ${img} == *"-run"* ]]; then
    cat bit.user.dockerfile >> ${dfile}
    cat bit.run.dockerfile >> ${dfile}
  elif [[ ${img} == *"-dev"* ]]; then
    cat bit.isrhub.dockerfile >> ${dfile}
    cat bit.user.dockerfile >> ${dfile}
    cat bit.run.dockerfile >> ${dfile}
    args=${bldargs[@]}
  else
    cat bit.user.dockerfile >> ${dfile}
  fi
  cat bit.tail.dockerfile >> ${dfile}
  time docker image build \
    --network=host \
    ${args[@]} \
    --build-arg USERNAME=${USER} \
    --build-arg USERID=$(id -u ${USER}) \
    --build-arg GROUPID=$(id -g ${USER}) \
    --file ${dfile} \
    --tag bpro/${img}:${gtag} .
done
docker image ls
