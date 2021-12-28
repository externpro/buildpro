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
