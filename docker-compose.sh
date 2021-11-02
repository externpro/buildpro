#!/usr/bin/env bash
cd "$( dirname "$0" )"
function build
{
  if  [[ -x .devcontainer/denv.sh ]]; then
    ./.devcontainer/denv.sh
    cat .env
    docker-compose build
  fi
}
function usage
{
  echo "`basename -- $0` usage:"
  echo " -h      display this help message"
  echo "         run the build container (no switches)"
  echo " -b      build docker image(s)"
}
if [ $# -eq 0 ]; then
  build
  docker-compose run --rm bpro
  exit 0
fi
while getopts "bh" opt
do
  case ${opt} in
    b )
      build
      exit 0
      ;;
    h )
      usage
      exit 0
      ;;
    \? )
      usage
      exit 0
      ;;
  esac
done
