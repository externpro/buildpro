#!/usr/bin/env bash
cd "$( dirname "$0" )"
source ./.devcontainer/funcs.sh
function sysreq
{
  gitlfsreq
  composereq
}
function usage
{
  echo "`basename -- $0` usage:"
  echo " -h      display this help message"
  echo "         run the build container (no switches)"
  echo " -b      build docker image(s)"
  echo " -d      run the develop container"
  echo " -r      run the runtime container"
}
if [ $# -eq 0 ]; then
  sysreq
  init
  docker-compose --profile pbld build
  docker-compose run --rm bld
  exit 0
fi
while getopts "bdhr" opt
do
  case ${opt} in
    b )
      init
      docker-compose --profile pbld --profile pdev --profile prun build
      exit 0
      ;;
    d )
      sysreq
      runreq
      init
      docker-compose --profile pdev up -d --build
      docker exec -it vantagedev bash
      docker-compose --profile pdev down
      exit 0
      ;;
    r )
      sysreq
      runreq
      init
      docker-compose --profile prun up -d --build
      docker exec -it vantagerun bash
      docker-compose --profile prun down
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
