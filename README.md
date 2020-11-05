# buildpro

build containers for [externpro](https://github.com/smanders/externpro) and projects that use externpro

## Table of Contents
- [Getting started with docker](#getting-started-with-docker)
  - [install and configure docker](#install-and-configure-docker)
  - [verify docker installation and configuration](#verify-docker-installation-and-configuration)
  - [short docker tutorial](#short-docker-tutorial)
- [Getting started with buildpro](#getting-started-with-buildpro)
  - [bprun usage examples](#bprun-usage-examples)

## Getting started with docker

### install and configure docker

Install Docker Engine https://docs.docker.com/engine/install/

An alternative is to install the `docker` snap package
* https://www.unixtutorial.org/how-to-install-docker-in-ubuntu-using-snap/
* https://github.com/docker-snap/docker-snap

If you don't want to preface the `docker` command with `sudo`, create a Unix group called `docker` and add users to it.
See "Post-installation steps for Linux" https://docs.docker.com/engine/install/linux-postinstall/ for details and warnings.

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

the commands `docker image` and `docker container` will display commands for managing images and containers

if `docker image rm hello-world` is attempted, there is an error that a container is using its referenced image,
so first remove the container, using it's randomly assigned name "romantic_torvalds" or the container ID
```
$ docker rm romantic_torvalds
$ docker image rm hello-world
```

## Getting started with buildpro

buildpro was designed to be run from a git repository (`docker tag` comes from `git tag`), so start by cloning the repo
```
$ git clone git://github.com/smanders/buildpro.git
$ cd buildpro
```
there are two main buildpro scripts
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
   -m arg  directory to mount to /srcdir in container (default: $HOME)
   -r arg  specify a repository (default: bpro/centos6-bld)
   -s      snap workaround: mount $HOME/tmp/.X11-unix to /tmp/.X11-unix
   -t arg  specify a repository tag (default: 20.9)
   -v      verbose (display 'docker container run' command)
   -x      X11 forwarding (container running on remote system via ssh [-X|-Y])
  ```

### bprun usage examples

* `-m arg` "mount" option
  * by default, `bprun` mounts your `$HOME` directory from the host system
    to `/srcdir` in the container
  * but you can specify what host directory to mount with the `-m` option,
    for example mounting `~/src` to `/srcdir`
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
    docker container run  --volume=/home/smanders/src/buildpro/image/scripts:/scripts
    --volume=/home/smanders:/srcdir --volume=/home/smanders/.ssh:/home/smanders/.ssh
    --volume=/tmp/.X11-unix:/tmp/.X11-unix --env=DISPLAY=bluepill:10.0 --network=bpnet
    --dns=172.17.0.1  --user=4793:100 --hostname=buildpro_working --rm -it
    bpro/centos6-bld:working shell
    ```
* `-x` "X11 forwarding" option
* `-s` "Snap" option
