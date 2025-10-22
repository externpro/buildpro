#!/bin/bash
cd "$( dirname "$0" )"
source $1
if [[ -z $REPOS || -z $SRCDIR ]]; then
  echo "$1 must define REPOS SRCDIR"
  exit
fi
if [ ! -d "$SRCDIR" ]; then
  mkdir --parents $SRCDIR
fi
pushd $SRCDIR >/dev/null
for repo in $REPOS
do
  dir=`echo ${repo} | rev | cut -d/ -f1 | rev`
  if [ ! -d "${dir}" ]
  then
    echo ==== ${repo} ====
    git clone ${repo}
    pushd ${dir} >/dev/null
    branch=$(git branch | sed -n -e 's/^\* \(.*\)/\1/p')
    if [ "${branch}" != "development" ]; then
      git ls-remote --exit-code --heads origin refs/heads/development >/dev/null && git checkout -b development origin/development
    fi
    if [ -f ".gitmodules" ]; then
      git submodule init
      git submodule update
    fi
    popd >/dev/null
  fi
done
popd >/dev/null
