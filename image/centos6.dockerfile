FROM centos:6
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
     centos-release-scl \
     gtk2-devel.x86_64 \
     libSM-devel.x86_64 \
     mesa-libGL-devel.x86_64 \
     mesa-libGLU-devel.x86_64 \
     ncurses-devel \
     redhat-lsb-core \
     wget \
     https://repo.ius.io/ius-release-el6.rpm \
     https://dl.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm \
  # https://www.softwarecollections.org/en/ and https://ius.io/
  && yum -y install --setopt=tsflags=nodocs \
     devtoolset-7-binutils \
     devtoolset-7-gcc \
     devtoolset-7-gcc-c++ \
     python27 \
     https://repo.ius.io/6/x86_64/packages/g/git224-2.24.3-1.el6.ius.x86_64.rpm \
  && yum clean all
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.4/cmake-3.17.4-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/
RUN mkdir /opt/extern
RUN wget -qO- "https://github.com/smanders/externpro/releases/download/20.08.1/externpro-20.08.1-gcc731-64-Linux.tar.xz" \
  | tar -xJ -C /opt/extern/
# set up volumes
VOLUME /scripts
VOLUME /srcdir
# enable scl binaries
ENV BASH_ENV="/scripts/scl_enable" \
    ENV="/scripts/scl_enable" \
    PROMPT_COMMAND=". /scripts/scl_enable"
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/scripts/entry.sh"]
