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
     graphviz \
     gtk2-devel.x86_64 \
     libSM-devel.x86_64 \
     mesa-libGL-devel.x86_64 \
     mesa-libGLU-devel.x86_64 \
     ncurses-devel \
     redhat-lsb-core \
     rpm-build \
     unixODBC-devel \
     wget \
     https://dl.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm \
     https://repo.ius.io/ius-release-el6.rpm \
  && yum -y install --setopt=tsflags=nodocs \
     cppcheck `#epel` \
     devtoolset-7-binutils `#scl` \
     devtoolset-7-gcc `#scl` \
     devtoolset-7-gcc-c++ `#scl` \
     lcov `#epel` \
     python27 `#scl` \
     https://repo.ius.io/6/x86_64/packages/g/git224-2.24.3-1.el6.ius.x86_64.rpm `#ius.io` \
  && yum clean all
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.4/cmake-3.17.4-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/
# CRTool
RUN mkdir /opt/extern && mkdir /opt/extern/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/20.07.1/CRTool-20.07.1.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/20.05.2/CRToolImpl-20.05.2.sh" \
  && chmod 755 CRTool*.sh \
  && /CRTool-20.07.1.sh --prefix=/opt/extern/CRTool --exclude-subdir \
  && /CRToolImpl-20.05.2.sh --prefix=/opt/extern --include-subdir \
  && rm /CRTool-20.07.1.sh \
  && rm /CRToolImpl-20.05.2.sh
# pros
ENV \
  XP_VER=20.08.1 \
  IP_VER=20.09.1 \
  WP_VER=20.06.1
# externpro
RUN wget -qO- "https://github.com/smanders/externpro/releases/download/$XP_VER/externpro-$XP_VER-gcc731-64-Linux.tar.xz" \
  | tar -xJ -C /opt/extern/ \
  && printf "lsb_release %s\n" "`lsb_release --description`" \
     >> /opt/extern/externpro-$XP_VER-gcc731-64-Linux/externpro_$XP_VER-gcc731-64.txt
# internpro
RUN wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/releases/download/$IP_VER/internpro-$IP_VER-gcc731-64-Linux.tar.xz" \
  | tar -xJ -C /opt/extern/
# webpro
RUN wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/releases/download/$WP_VER/webpro-$WP_VER-gcc731-64-Linux.tar.xz" \
  | tar -xJ -C /opt/extern/
# set up volumes
VOLUME /scripts
VOLUME /srcdir
# enable scl binaries, add CRTool to PATH, set USER
ENV BASH_ENV="/scripts/scl_enable" \
    ENV="/scripts/scl_enable" \
    PROMPT_COMMAND=". /scripts/scl_enable" \
    PATH=/opt/extern/CRTool:$PATH \
    USER=$USERNAME
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/scripts/entry.sh"]
