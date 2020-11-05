#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no .)" ]; then
  gtag=working
elif [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
fi
dodashu=true
host isrhub.usurf.usu.edu | grep "not found" >/dev/null && dodashu=false
render()
{
  sed_str="
    s!%%BP_REPO%%!${img}!g;
    s!%%BP_TAG%%!${gtag}!g;
  "
  sed -r "${sed_str}" $1
}
## bldargs
bldargs=()
pkgver=(\
  "CRT_VER=20.10.1"\
  "CRW_VER=20.07.1"\
  "SDK_VER=v3.2.0.0"\
  "IP_VER=20.10.1"\
  "WP_VER=20.10.3"\
  )
for p in ${pkgver[@]}; do
  bldargs+=("--build-arg $p")
done
## runargs
runargs=()
flags=(\
  "ODBC_INI=true"\
  )
for f in ${flags[@]}; do
  runargs+=("--build-arg $f")
done
# download/pull or build images
for img in centos6-bld centos7-run centos7-bld
do
  pkg=ghcr.io/smanders/buildpro/${img}:${gtag}
  docker pull ${pkg}
  if [[ "$(docker images -q ${pkg} 2>/dev/null)" == "" || "${gtag}" == "working" ]]
  then
    time docker image build \
      --network=host \
      --file ${img}.dockerfile \
      --tag ${pkg} .
  fi
  if ${dodashu}; then
    if [[ ${img} == *"-bld"* ]]; then
      args=${bldargs[@]}
    elif [[ ${img} == *"-run"* ]]; then
      args=${runargs[@]}
    else
      args=()
    fi
    render template.dockerfile > .dockerfiles/${img}-u.dockerfile
    time docker image build \
      --network=host \
      ${args[@]} \
      --build-arg USERNAME=${USER} \
      --build-arg USERID=$(id -u ${USER}) \
      --file .dockerfiles/${img}-u.dockerfile \
      --tag bpro/${img}:${gtag} .
  else
    echo "isrhub.usurf.usu.edu not accessible"
  fi
done
docker image ls
