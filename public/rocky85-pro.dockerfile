FROM ghcr.io/smanders/rockylinux:8.5
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# dnf repositories
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     coreutils-common \
     git \
     gtk3-devel \
     mesa-libGL-devel \
     mesa-libGLU-devel \
     python3 \
     redhat-lsb-core \
     sudo \
     vim \
     wget \
  && dnf clean all
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     gcc-toolset-9-binutils \
     gcc-toolset-9-gcc \
     gcc-toolset-9-gcc-c++ \
     gcc-toolset-9-gdb \
     gcc-toolset-9-libasan-devel \
     gcc-toolset-9-libtsan-devel \
     gcc-toolset-9-make \
  && dnf clean all
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
ENV GCC_VER=gcc921 \
    PATH="/opt/rh/gcc-toolset-9/root/usr/bin:${PATH}" \
    EXTERN_DIR=/opt/extern \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
