#!/usr/bin/env bash
cd "$( dirname "$0" )"
gtag=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no)" ] || [[ ${gtag} == *"-g"* ]]; then
  gtag=latest
fi
# download/pull images (ghimg.sh builds them)
for img in centos7-run centos7-pro centos7-bld centos7-dev
do
  dfile=../.devcontainer/${img}.dockerfile
  awk -v r="${img}" -v t="${gtag}" '{gsub(/%BP_REPO%/,r);gsub(/%BP_TAG%/,t)} 1' bit.head.dockerfile > ${dfile}
  if [[ ${img} == *"-bld"* ]]; then
    cat bit.offline.dockerfile >> ${dfile}
    cat bit.isrhub.dockerfile >> ${dfile}
    cat bit.user.dockerfile >> ${dfile}
  elif [[ ${img} == *"-run"* ]]; then
    cat bit.user.dockerfile >> ${dfile}
    cat bit.run.dockerfile >> ${dfile}
  elif [[ ${img} == *"-dev"* ]]; then
    cat bit.isrhub.dockerfile >> ${dfile}
    cat bit.user.dockerfile >> ${dfile}
    cat bit.run.dockerfile >> ${dfile}
  else
    cat bit.user.dockerfile >> ${dfile}
  fi
  cat bit.tail.dockerfile >> ${dfile}
done
