FROM centos:7
LABEL maintainer="smanders"
SHELL ["/bin/bash", "-c"]
# Create non-root user:group and generate a home directory to support SSH
ARG USERNAME
ARG USERID
USER 0
RUN adduser --uid ${USERID} ${USERNAME}
# install software build system inside docker
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     gtk2.x86_64 \
     libSM.x86_64 \
     make \
     unixODBC \
     wget \
  && yum clean all
# cmake
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.5/cmake-3.17.5-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/local/
# set up volumes
VOLUME /scripts
VOLUME /srcdir
# set USER
ENV USER=$USERNAME
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/scripts/entry.sh"]
