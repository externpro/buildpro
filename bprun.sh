#!/usr/bin/env bash
cd "$( dirname "$0" )"
function usage
{
  echo "`basename -- $0` usage:"
  echo " -d      runtime (bpro/centos7-run) and database (mysqlpro) containers"
  echo " -h      display this help message"
  echo " -m arg  directory to mount to /srcdir in container (default: \$HOME)"
  echo " -r arg  specify a repository (default: $REPO)"
  echo " -s      snap workaround: mount \$HOME/tmp/.X11-unix to /tmp/.X11-unix"
  echo " -t arg  specify a repository tag (default: $TAG)"
  echo " -v      verbose (display 'docker container run' command)"
  echo " -x      X11 forwarding (container running on remote system via ssh [-X|-Y])"
}
docker network inspect bpnet >/dev/null 2>&1 || \
  docker network create --driver bridge --opt com.docker.network.driver.mtu=9000 bpnet >/dev/null 2>&1
db_container_name=mysqlpro
function dbinit
{
  # https://dev.mysql.com/doc/refman/8.0/en/docker-mysql-getting-started.html
  # https://severalnines.com/database-blog/mysql-docker-containers-understanding-basics
  if [ ! "$(docker ps -q -f name=${db_container_name})" ]; then
    if [ "$(docker ps -aq -f status=exited -f name=${db_container_name})" ]; then
      docker rm ${db_container_name}
    fi
    db_root_passwd=mysqlroot
    docker run --name=${db_container_name} \
      --restart on-failure \
      --detach \
      --env="MYSQL_ROOT_PASSWORD=${db_root_passwd}" \
      --network=bpnet \
      mysql/mysql-server:8.0.21 \
        --innodb-buffer-pool-size=2G \
        --innodb-flush-log-at-trx-commit=2 \
        --disable-log-bin \
        --sql-mode=STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION \
      >/dev/null 2>&1
    echo "${db_container_name} starting... waiting until status reports 'healthy'"
    while [ ! "$(docker ps -f name=${db_container_name} | grep healthy)" ]; do
      sleep 3
    done
    echo "initializing ${db_container_name}"
    docker exec -it ${db_container_name} mysql -uroot -p${db_root_passwd} \
      -e "CREATE DATABASE IF NOT EXISTS mock_midb;" \
      -e "CREATE DATABASE IF NOT EXISTS mock_users;" \
      -e "CREATE DATABASE IF NOT EXISTS mock_events;" \
      -e "CREATE DATABASE IF NOT EXISTS midb;" \
      -e "CREATE DATABASE IF NOT EXISTS users;" \
      -e "CREATE DATABASE IF NOT EXISTS eventsdb;" \
      -e "CREATE USER IF NOT EXISTS 'vantage'@'%' IDENTIFIED BY 'TestDB4me!';" \
      -e "GRANT ALL ON *.* TO 'vantage'@'%';" \
      -e "FLUSH PRIVILEGES;"
    # NOTES: to verify settings, etc
    # docker [stop|retart|start|rm] ${db_container_name}
    # docker inspect ${db_container_name} | grep IPAddress
    # ip a | grep docker | grep inet
    # in link'ed container:
    #  cat /etc/hosts
    #  cat /etc/odbc.ini
    #  isql mock_midb_dsn
    # docker exec -it ${db_container_name} mysql -uroot -p
    # mysql> SELECT @@innodb_buffer_pool_size;
    # mysql> SELECT @@innodb_flush_log_at_trx_commit;
    # mysql> SELECT @@sql_mode;
    # mysql> quit;
    # docker exec -it ${db_container_name} bash
    # mysqladmin -uroot -p variables
    # mysqld --verbose --help (to see default values)
  fi
  if [ ! "$(docker container inspect -f '{{.State.Status}}' ${db_container_name})" == "running" ]; then
    echo "${db_container_name} not running"
    exit
  fi
}
CMD=shell
MOUNT=$HOME
REPO=bpro/centos6-bld
SNAP=
TAG=`git describe --tags`
if [ -n "$(git status --porcelain --untracked=no)" ]; then
  TAG=working
elif [[ ${TAG} == *"-g"* ]]; then
  TAG=latest
fi
CONTAINER_HOSTNAME=buildpro_${TAG}
VERBOSE=
DOCKER_HOST=$(ip -4 addr show docker0 | grep -Po 'inet \K[\d.]+')
XARG="--env=DISPLAY=${DISPLAY}"
while getopts ":c:dhm:r:st:vx" opt
do
  case ${opt} in
    c )
      CMD=$OPTARG
      ;;
    d )
      REPO=bpro/centos7-run
      dbinit
      ;;
    h )
      usage
      exit 0
      ;;
    m )
      MOUNT=$OPTARG
      ;;
    r )
      REPO=$OPTARG
      ;;
    s )
      SNAP=${HOME}
      ;;
    t )
      TAG=$OPTARG
      CONTAINER_HOSTNAME=buildpro_${TAG}
      ;;
    v )
      VERBOSE=true
      ;;
    x )
      DISPLAY_SCREEN=$(echo $DISPLAY | cut -d: -f2)
      DISPLAY_NUM=$(echo ${DISPLAY_SCREEN} | cut -d. -f1)
      MAGIC_COOKIE=$(xauth list ${DISPLAY} | awk '{print $3}')
      XAUTH=/tmp/.X11-unix/docker.xauth
      touch ${XAUTH}
      xauth -f ${XAUTH} add ${DOCKER_HOST}:${DISPLAY_NUM} . ${MAGIC_COOKIE}
      XARG="--env=DISPLAY=${DOCKER_HOST}:${DISPLAY_SCREEN} --env=XAUTHORITY=${XAUTH}"
      ;;
    \? )
      echo "Invalid option: $OPTARG" 1>&2
      usage
      exit 1
      ;;
    : )
      echo "Invalid option: $OPTARG requires an argument" 1>&2
      usage
      exit 1
      ;;
  esac
done
shift $((OPTIND -1))
REMAINING_ARGS="$@" # args after --
RUN_ARGS="\
 ${REMAINING_ARGS}\
 --volume=$(pwd)/image/scripts:/scripts\
 --volume=$MOUNT:/srcdir\
 --volume=$HOME/.ssh:/home/${USER}/.ssh\
 --volume=${SNAP}/tmp/.X11-unix:/tmp/.X11-unix\
 ${XARG}\
 --network=bpnet --dns=${DOCKER_HOST}\
 --user=$(id -u ${USER}):$(id -g ${USER})\
 --hostname=${CONTAINER_HOSTNAME}\
 --rm -it ${REPO}:${TAG}\
 ${CMD}"
if [ $VERBOSE ]; then
  echo "docker container run${RUN_ARGS}"
fi
if [[ "$(docker images -q ${REPO}:${TAG} 2>/dev/null)" == "" ]]; then
  ./image/bpimg.sh
  echo "'docker image build' complete... 'docker container run' starting"
fi
docker container run ${RUN_ARGS}
