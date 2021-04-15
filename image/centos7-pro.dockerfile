FROM centos:7
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# yum repositories
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     centos-release-scl \
     gtk2-devel.x86_64 \
     mesa-libGL-devel.x86_64 \
     mesa-libGLU-devel.x86_64 \
     redhat-lsb-core \
     sudo \
     vim \
     wget \
     https://repo.ius.io/ius-release-el7.rpm \
  && yum clean all
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     libtsan-7.3.1 \
     devtoolset-7-binutils `#scl` \
     devtoolset-7-gcc `#scl` \
     devtoolset-7-gcc-c++ `#scl` \
     devtoolset-7-libasan-devel `#scl` \
     devtoolset-7-libtsan-devel `#scl` \
     devtoolset-7-gdb `#scl` \
     git224 `#ius.io` \
  && echo "exclude=libtsan" >> /etc/yum.conf \
  && yum clean all
# cmake
RUN export CMK_VER=3.17.5 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-Linux-x86_64.tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
# environment: gcc version, enable scl binaries
ENV GCC_VER=gcc731 \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
