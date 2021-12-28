# buildpro

build containers for [externpro](https://github.com/smanders/externpro)
and projects that use externpro

## Table of Contents
- [Getting started with docker](#getting-started-with-docker)
  - [install and configure docker](#install-and-configure-docker)
  - [verify docker installation and configuration](#verify-docker-installation-and-configuration)
  - [short docker tutorial](#short-docker-tutorial)
  - [useful docker commands](#useful-docker-commands)
- [Using buildpro](#using-buildpro)
- [Getting started with buildpro](#getting-started-with-buildpro)
  - [bprun usage examples](#bprun-usage-examples)
  - [additional configuration](#additional-configuration)
    - [network](#network)
    - [dns](#dns)
    - [X11 forwarding](#X11-forwarding)
    - [network performance tuning](#network-performance-tuning)
    - [database](#database)

## Getting started with docker

### install and configure docker

Install Docker Engine https://docs.docker.com/engine/install/

If you don't want to preface the `docker` command with `sudo`, create a Unix group
called `docker` and add users to it. See "Post-installation steps for Linux"
https://docs.docker.com/engine/install/linux-postinstall/ for details and warnings.

### verify docker installation and configuration
(all examples below now assume you have configured your system to run `docker` without `sudo`)
```
$ docker run hello-world
```
you should see a "Hello from Docker!" message with additional details

### short docker tutorial
image vs container https://stackify.com/docker-image-vs-container-everything-you-need-to-know/
* in other virual machine environments, images would be called something like "snapshots"
* Docker images can't ever change -- once you've made one, you can delete it, but you can't modify it
* if you need a new version of the snapshot, you create an entirely new image
* if a Docker image is a digital photograph, a Docker container is like a printout of that photograph
* a container is an "instance" of the image
* each Docker container runs separately, and you can modify the container while it's running

```
$ docker image ls
REPOSITORY    TAG      IMAGE ID       CREATED         SIZE
hello-world   latest   bf756fb1ae65   10 months ago   13.3kB

$ docker container ps -a
CONTAINER ID   IMAGE         COMMAND    CREATED         STATUS                     PORTS   NAMES
feaadc8cc2b1   hello-world   "/hello"   2 minutes ago   Exited (0) 2 minutes ago           romantic_torvalds
```
shorter versions of these commands
* `docker images` instead of `docker image ls`
* `docker ps -a` instead of `docker container ps -a`
* a lot of docker commands assume "container"
  * `docker run` is short for `docker container run`
  * `docker rm` is short for `docker container rm`

the commands `docker image` and `docker container` will display commands for managing
images and containers

if `docker image rm hello-world` is attempted, there is an error that a container is
using its referenced image, so first remove the container, using it's randomly assigned
name "romantic_torvalds" or the container ID
```
$ docker rm romantic_torvalds
$ docker image rm hello-world
```
### useful docker commands
```
$ docker images
$ docker ps -a
$ docker [stop|restart|start|rm|logs|inspect] <container_name>
$ docker inspect <container_name> | grep -i ipaddress
$ docker exec -it <container_name> bash
```

## Using buildpro

To use buildpro docker images
* copy the contents of the [.devcontainer](.devcontainer) directory to a `.devcontainer` directory
  in the root of the project wishing to use buildpro images
* create symbolic links to the `compose.*.[sh|yml]` file pair suitable for the project
  ```
  ln -s .devcontainer/compose.bld.sh docker-compose.sh
  ln -s .devcontainer/compose.bld.yml docker-compose.yml
  ```
* `./docker-compose.sh -h` to display a help message showing usage and options

## Getting started with buildpro

buildpro was designed to be run from a git repository (`docker tag` comes from `git tag`),
so start by cloning the repo
```
$ git clone git://github.com/smanders/buildpro.git
$ cd buildpro
```
there are two main buildpro scripts: `bpimg.sh` and `bprun.sh`
* [bpimg.sh](image/bpimg.sh)
  * located in the [image/](image) directory
  * the `image/` directory contains dockerfiles and other files to support building docker images
  * main task is to run `docker image build`
  * running the `bpimg.sh` script will
    * download/pull the base open source packages `ghcr.io/smanders/buildpro/` from
      https://github.com/smanders?tab=packages&repo_name=buildpro 
    * build the `bpro/` images that require access to the internal isrhub (internpro, etc),
      which are based on the open source images
  ```
  $ docker images
  REPOSITORY                              TAG       IMAGE ID            CREATED             SIZE
  bpro/centos7-bld                        20.9      b92aaf04c4f7        12 days ago         7.29GB
  bpro/centos7-run                        20.9      4e2e19507c18        12 days ago         441MB
  bpro/centos6-bld                        20.9      066364e38517        12 days ago         7.54GB
  ghcr.io/smanders/buildpro/centos7-bld   20.9      f41cabdc77e6        2 weeks ago         3.87GB
  ghcr.io/smanders/buildpro/centos6-bld   20.9      94fe6b48f41c        3 weeks ago         4.12GB
  ghcr.io/smanders/buildpro/centos7-run   20.9      d28fd6963d52        5 weeks ago         439MB
  centos                                  7         7e6257c9f8d8        2 months ago        203MB
  centos                                  6         d0957ffdf8a2        20 months ago       194MB
  ```
  * NOTE regarding image naming conventions
    * the `bpro/` images are the ones meant to be run
      * all other images are "base" images used to build up to the `bpro/` images
    * `-bld` for a build container
      * with all the packages necessary to build software
      * unit tests that don't require database should run fine in a build container
    * `-run` for a runtime container
      * with all the packages necessary to run unit tests and Autotest (which require database)
* [bprun.sh](bprun.sh)
  * located in the root directory of buildpro
  * main task is to run `docker container run`
  * running the `bprun.sh` script will call `bpimg.sh` if the image it's attempting
    to create a container from doesn't exist
  * the `-h` option shows the usage options
  ```
  $ ./bprun.sh -h
  bprun.sh usage:
   -d      runtime (bpro/centos7-run) and database (mysqlpro) containers
   -h      display this help message
   -m arg  directory to mount to /bpvol in container (default: $HOME)
   -n      use --network=host instead of --network=bpnet --dns=$DOCKER_HOST
   -r arg  specify a repository (default: bpro/centos6-bld)
   -s      snap workaround: mount $HOME/tmp/.X11-unix to /tmp/.X11-unix
   -t arg  specify a repository tag (default: 20.9)
   -v      verbose (display 'docker container run' command)
  ```

### bprun usage examples

* `-m arg` "mount" option
  * by default, `bprun` mounts your `$HOME` directory from the host system
    to `/bpvol` in the container
  * but you can specify what host directory to mount with the `-m` option,
    for example mounting `~/src` to `/bpvol`
  * example: `$ ./bprun -m ~/src`
* `-r arg` "repo" option
  * by default, `bprun` uses the repository listed as the default by `bprun -h`
  * but you can specify a specific repository image to create a container from
  * NOTE: the default will likely change as we move to newer OS releases
  * example: `$ ./bprun -r bpro/centos7-bld`
* `-t arg` "tag" option
  * by default, `bprun` uses the tag listed as the default by `bprun -h`
  * but you can specify a specific repository image tag to create a container from
  * NOTE: the default will change depending on the state of the buildpro git repository
    * if you're on the `master` branch or a tagged release, default will match (ex: `20.9`)
    * if you're on the `dev` branch, not a tagged release, default will be `latest`
    * if you have local modifications to the repo, default will be `working`
  * example: `$ ./bprun -t 20.9`
* `-d` "database" option
  * changes the default repository (`-r` option) to a `-run` container
  * starts up an additional database container (currently named `mysqlpro`)
    * also downloads/pulls mysql image, if not already available
    * NOTE: mysql/mysql-server version (tag) used is in the bprun.sh script
  * all the packages and environment necessary for unit tests and Autotest which require database
  * example: `$ ./bprun -d`
* `-v` "verbose" option
  * because sometimes it's useful to see the `docker container run` command
  * example: `$ ./bprun -v`
    ```
    docker container run --volume=/home/smanders:/bpvol
    --volume=/home/smanders/.ssh:/home/smanders/.ssh
    --volume=/tmp/.X11-unix:/tmp/.X11-unix
    --env=DISPLAY=bluepill:10.0
    --network=bpnet --dns=172.17.0.1  --user=4455:100 --hostname=buildpro_latest --rm -it
    bpro/centos6-bld:working
    ```
### additional configuration

### network
* the `bprun` script creates a user-defined bridge network named `bpnet`
  ```
  $ docker network ls
  NETWORK ID      NAME         DRIVER       SCOPE
  4f95e134e980    bpnet        bridge       local
  106bd2fa866e    bridge       bridge       local
  ccb360221365    host         host         local
  b2a1087b5854    none         null         local
  ```
* the `docker container run` command generated by `bprun` (shown above) specifies
  the network `--network=bpnet`
* from https://docs.docker.com/network/
  > User-defined bridge networks are best when you need multiple containers to
    communicate on the same Docker host
* other benefits and differences bewteen user-defined bridges and the default bridge
  are detailed in the docker docs
  https://docs.docker.com/network/bridge/
  * user-defined bridges provide automatic DNS resolution between containers
  * user-defined bridges provide better isolation
  * containers can be attached and detached from user-defined networks on the fly
  * each user-defined network creates a configurable bridge
    * the `docker network create` command issued by `bprun` includes
      `--opt com.docker.network.driver.mtu=9000`
      to "turn on" [jumbo frames](https://en.wikipedia.org/wiki/Jumbo_frame)
    * https://docs.docker.com/engine/reference/commandline/network_create/#bridge-driver-options
    * this can be verified inside the container
      ```
      $ ip link show | grep mtu
      1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
      500: eth0@if501: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 9000 qdisc noqueue state UP
      ```

#### dns
* the `docker container run` command generated by `bprun` (show above)
  specifies the dns `--dns=172.17.0.1`
* `172.17.0.1` is the ip address of `docker0`
  ```
  $ ip -4 addr show docker0
  5: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN group default
      inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
         valid_lft forever preferred_lft forever
  $ ip a | grep docker | grep inet
      inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
  ```
* if the container is unable to use dns, for example:
  ```
  $ ping github.com
  ping: unknown host github.com
  ```
  * "DNS not working within docker containers when host uses dnsmasq
    and Google's DNS server are firewalled?" https://stackoverflow.com/a/50001940
  * on the host system, create a new file with the line `listen-address=172.17.0.1`
    and restart the network-manager
    ```
    $ sudo vi /etc/NetworkManager/dnsmasq.d/docker-bridge.conf
    listen-address=172.17.0.1
    $ sudo service network-manager restart
    ```
* if the container is still unable to use dns, the `bprun.sh -n` option will replace
  the default `--network=bpnet --dns=172.17.0.1` with `--network=host`
  * hopefully this is a temporary work-around until dns works for different setups
  * see the [network](#network) section for benefits of user-defined networks and
    https://docs.docker.com/network/host/ for details of host networking

#### X11 forwarding
* if you're running `bprun` on a remote system you've connected to via `ssh -X` or `ssh -Y`
  the script should automatically detect this case and will do additional configuration and
  add/modify parameters to the `docker container run` command so that X display from the running
  container will (hopefully) work as expected
* example: `$ ./bprun -v`
  ```
  docker container run --volume=/home/smanders:/bpvol
  --volume=/home/smanders/.ssh:/home/smanders/.ssh
  --volume=/tmp/.X11-unix:/tmp/.X11-unix
  --env=DISPLAY=172.17.0.1:10.0 --env=XAUTHORITY=/tmp/.X11-unix/docker.xauth
  --network=bpnet --dns=172.17.0.1  --user=4455:100 --hostname=buildpro_latest --rm -it
  bpro/centos6-bld:working
  ```
* NOTE: the `-bld` images include the `xeyes` package, which can be run (`$ xeyes &`) from the
  container to verify X11 forwarding is working as expected
* TIP: if you get a "can't open display" error trying to run X applications, you may need
  to change the `X11UseLocahost` option in `/etc/ssh/sshd_config` to `no` (and restart sshd)

#### network performance tuning
* with `/etc/sysctl.conf` you can configure various linux kernel networking settings
* some of these settings are required to be modified for Autotest
  (in the runtime container) to succeed
* some `--sysctl` settings can be applied to a container in the `docker container run` command,
  but I found that none of the required changes could be done this way
  (or at least in my attempts with Docker version 18.09.7)
  https://docs.docker.com/engine/reference/commandline/run/#configure-namespaced-kernel-parameters-sysctls-at-runtime
* these settings need to be applied to the host, then the docker container
  (which shares the host kernel) will have the required settings
* https://unix.stackexchange.com/questions/404387/how-to-sysctl-net-related-config-in-docker-container/455193
* the `/etc/sysctl.d/README` explains the directory's relation to `/etc/sysctl.conf` and mentions
  > After making any changes, please run "service procps start"
  * I believe they meant `restart` instead of `start`
* the script [image/check-bpnet-perform.sh](image/check-bpnet-perform.sh) will help
  you to check the values before applying the changes in
  [image/90-bpnet-perform.conf](image/90-bpnet-perform.conf) -- in case you'd like to
  ever go back to the original, default values
  ```
  $ ./image/check-bpnet-perform.sh
  net.core.rmem_max = 212992
  net.core.wmem_max = 212992
  net.core.rmem_default = 212992
  net.core.wmem_default = 212992
  net.ipv4.tcp_rmem = 4096	87380	6291456
  net.ipv4.tcp_wmem = 4096	16384	4194304
  net.ipv4.tcp_mem = 383520	511360	767040
  net.ipv4.udp_rmem_min = 4096
  net.ipv4.udp_wmem_min = 4096
  net.ipv4.udp_mem = 767040	1022720	1534080

  $ sudo cp image/90-bpnet-perform.conf /etc/sysctl.d/
  $ sudo service procps restart

  $ ./image/check-bpnet-perform.sh
  net.core.rmem_max = 8388608
  net.core.wmem_max = 8388608
  net.core.rmem_default = 8388608
  net.core.wmem_default = 8388608
  net.ipv4.tcp_rmem = 94096	987380	8388608
  net.ipv4.tcp_wmem = 94096	987380	8388608
  net.ipv4.tcp_mem = 8388608	8388608	8388608
  net.ipv4.udp_rmem_min = 8388608
  net.ipv4.udp_wmem_min = 8388608
  net.ipv4.udp_mem = 8388608	8388608	8388608
  ```

#### database
* https://dev.mysql.com/doc/refman/8.0/en/docker-mysql-getting-started.html
* https://severalnines.com/database-blog/mysql-docker-containers-understanding-basics
* verify start-up configuration is as-expected (changed from defaults)
  ```
  $ docker exec -it mysqlpro mysql -uvantage -p
  mysql> SELECT @@innodb_buffer_pool_size;
  mysql> SELECT @@innodb_flush_log_at_trx_commit;
  mysql> SELECT @@sql_mode;
  mysql> quit;
  ```
* some useful mysql commands to get configuration correct
  ```
  $ docker exec -it mysqlpro bash
  $ mysqld --verbose --help (to see default values)
  $ mysqladmin -uroot -p variables
  ```
* troubleshooting the runtime/database container connection
  ```
  $ ./bprun -d
  $ cat ~/.odbc.ini
  $ cat /etc/odbcinst.ini
  $ isql mock_midb_dsn
  ```
