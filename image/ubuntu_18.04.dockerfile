FROM ubuntu:18.04
LABEL maintainer="smanders"
SHELL ["/bin/bash", "-c"]
VOLUME /scripts
VOLUME /srcdir
# install software build system inside docker
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive \
     apt-get -y --quiet --no-install-recommends install \
     build-essential \
     ca-certificates \
     git \
     libglu1-mesa-dev \
     libgtk-3-dev \
     libncurses5-dev \
     lsb-release \
     python-dev \
     wget \
  && apt-get -y autoremove \
  && apt-get clean autoclean \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.4/cmake-3.17.4-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/
# create non-root user, add to sudoers
ARG USERNAME
ARG USERID
RUN adduser --disabled-password --gecos '' --uid ${USERID} ${USERNAME} \
  && adduser ${USERNAME} sudo \
  && echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# run bash script and process the input command
ENTRYPOINT [ "/bin/bash", "/scripts/entry.sh"]
