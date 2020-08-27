FROM centos/devtoolset-7-toolchain-centos7
LABEL maintainer="smanders"
SHELL ["/bin/bash", "-c"]
# Create non-root user:group and generate a home directory to support SSH
ARG USERNAME
ARG USERID
USER 0
RUN adduser --uid ${USERID} ${USERNAME}
# install software build system inside docker
RUN yum update -y \
  && yum install -y --setopt=tsflags=nodocs \
     git \
     gtk2-devel.x86_64 \
     libSM-devel.x86_64 \
     mesa-libGL-devel.x86_64 \
     mesa-libGLU-devel.x86_64 \
     ncurses-devel \
     redhat-lsb-core \
     wget \
  && yum clean all -y
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.4/cmake-3.17.4-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# set up volumes
VOLUME /scripts
VOLUME /srcdir
# run bash script and process the input command
ENTRYPOINT [ "/bin/bash", "/scripts/entry.sh"]
