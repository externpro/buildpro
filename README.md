# buildpro

build containers for externpro and projects that use externpro

## Table of Contents
- [Getting started with docker](#getting-started-with-docker)
  - [install and configure docker](#install-and-configure-docker)
  - [verify docker installation and configuration](#verify-docker-installation-and-configuration)
  - [short docker tutorial](#short-docker-tutorial)

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
