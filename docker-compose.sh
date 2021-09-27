#!/usr/bin/env bash
cd "$( dirname "$0" )"
if  [[ -x .devcontainer/denv.sh ]]; then
  ./.devcontainer/denv.sh
  cat .env
  docker-compose build
  docker-compose run --rm bpro
fi
