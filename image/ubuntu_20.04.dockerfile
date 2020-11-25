FROM ubuntu:20.04
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# install buildpro script-ready system
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive \
     apt-get -y --quiet --no-install-recommends install \
     apt-transport-https \
     ca-certificates \
     curl \
     git \
     gnupg-agent \
     lsb-release \
     software-properties-common \
     sudo \
     wget \
  && apt-get -y autoremove \
  && apt-get clean autoclean \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -
RUN add-apt-repository \
  "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) \
  stable"
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive \
     apt-get -y --quiet --no-install-recommends install \
     docker-ce \
     docker-ce-cli \
     containerd.io \
  && apt-get -y autoremove \
  && apt-get clean autoclean \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
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
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
