FROM ghcr.io/smanders/centos:7
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
     gtk3-devel \
     mesa-libGL-devel \
     mesa-libGLU-devel \
     redhat-lsb-core \
     sudo \
     vim \
     wget \
     https://repo.ius.io/ius-release-el7.rpm \
  && yum clean all
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     libtsan-9.3.1 \
     devtoolset-9-binutils `#scl` \
     devtoolset-9-gcc `#scl` \
     devtoolset-9-gcc-c++ `#scl` \
     devtoolset-9-libasan-devel `#scl` \
     devtoolset-9-libtsan-devel `#scl` \
     devtoolset-9-gdb `#scl` \
     git236 `#ius.io` \
     rh-python36 `#scl` \
  && echo "exclude=libtsan" >> /etc/yum.conf \
  && yum clean all
# cmake
RUN export CMK_VER=3.24.2 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-$(uname -s)-$(uname -m).tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
# environment: gcc version, enable scl binaries
ENV GCC_VER=gcc931 \
    PATH="/opt/rh/devtoolset-9/root/usr/bin:${PATH}" \
    EXTERN_DIR=/opt/extern \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
