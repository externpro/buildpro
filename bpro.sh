#!/usr/bin/env bash
cd "$( dirname "$0" )"
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
      mysql/mysql-server:8.0.21 \
        --innodb-buffer-pool-size=2G \
        --innodb-flush-log-at-trx-commit=2 \
        --disable-log-bin \
        --sql-mode=STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION
    echo "${db_container_name} starting... waiting until status reports 'healthy'"
    while [ ! "$(docker ps -f name=${db_container_name} | grep healthy)" ]; do
      sleep 3
    done
    echo "initializing ${db_container_name}"
    docker exec -it ${db_container_name} mysql -uroot -p${db_root_passwd} \
      -e "CREATE DATABASE mock_midb;" \
      -e "CREATE DATABASE mock_users;" \
      -e "CREATE DATABASE mock_events;" \
      -e "CREATE USER 'vantage'@'%' IDENTIFIED BY 'MockDB4me!';" \
      -e "GRANT ALL ON *.* TO 'vantage'@'%';" \
      -e "FLUSH PRIVILEGES;"
    # NOTES: to verify settings, etc
    # docker [stop|retart|start|rm] ${db_container_name}
    # docker inspect ${db_container_name} | grep IPAddress
    # ip a | grep docker | grep inet
    # cat /etc/hosts (in link'ed container)
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
DB=
MOUNT=$HOME
NETWORK=
REPO=buildpro/centos6-bld
TAG=`git describe --tags`
if [ -n "$(git status --porcelain)" ]; then
  TAG=${TAG}-dr
fi
CONTAINER_HOSTNAME=buildpro_${TAG}
VERBOSE=
XARG="--env=DISPLAY=${DISPLAY}"
while getopts ":c:dm:nr:t:vx" opt
do
  case ${opt} in
    c )
      CMD=$OPTARG
      ;;
    d )
      DB="--link ${db_container_name}:mysqldocker"
      REPO=buildpro/centos7-run
      dbinit
      ;;
    m )
      MOUNT=$OPTARG
      ;;
    n )
      NETWORK="--volume=$HOME/.ssh:/home/${USER}/.ssh --net=host"
      ;;
    r )
      REPO=$OPTARG
      ;;
    t )
      TAG=$OPTARG
      CONTAINER_HOSTNAME=buildpro_${TAG}
      ;;
    v )
      VERBOSE=true
      ;;
    x )
      DOCKER_HOST=$(ip -4 addr show docker0 | grep -Po 'inet \K[\d.]+')
      DISPLAY_SCREEN=$(echo $DISPLAY | cut -d: -f2)
      DISPLAY_NUM=$(echo ${DISPLAY_SCREEN} | cut -d. -f1)
      MAGIC_COOKIE=$(xauth list ${DISPLAY} | awk '{print $3}')
      XAUTH=/tmp/.docker.xauth
      touch ${XAUTH}
      xauth -f ${XAUTH} add ${DOCKER_HOST}:${DISPLAY_NUM} . ${MAGIC_COOKIE}
      XARG="--env=DISPLAY=${DOCKER_HOST}:${DISPLAY_SCREEN}
        --volume=${XAUTH}:${XAUTH} --env=XAUTHORITY=${XAUTH}"
      ;;
    \? )
      echo "Invalid option: $OPTARG" 1>&2
      exit
      ;;
    : )
      echo "Invalid option: $OPTARG requires an argument" 1>&2
      exit
      ;;
  esac
done
shift $((OPTIND -1))
REMAINING_ARGS="$@" # args after --
RUN_ARGS="\
 ${REMAINING_ARGS}\
 --volume=$(pwd)/image/scripts:/scripts\
 --volume=$MOUNT:/srcdir\
 ${NETWORK}\
 --volume=/tmp/.X11-unix:/tmp/.X11-unix\
 ${XARG} ${DB}\
 --user=$(id -u ${USER}):$(id -g ${USER})\
 --hostname=${CONTAINER_HOSTNAME}\
 --rm -it ${REPO}:${TAG}\
 ${CMD}"
if [ $VERBOSE ]; then
  echo "docker container run${RUN_ARGS}"
fi
docker container run ${RUN_ARGS}
