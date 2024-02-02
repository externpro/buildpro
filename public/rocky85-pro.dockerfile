FROM ghcr.io/externpro/rockylinux:8.5
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/externpro/buildpro
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
     python3-devel \
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
# Dockerfile.vim
RUN export DVIM_VER=21.09.06 \
  && export DVIM_SYS=/usr/share/vim/vimfiles \
  && export DVIM_DL=releases/download/${DVIM_VER}/Dockerfile.vim-${DVIM_VER}.tar.xz \
  && wget -qO- "https://github.com/smanders/Dockerfile.vim/${DVIM_DL}" | tar --no-same-owner -xJ -C ${DVIM_SYS} \
  && unset DVIM_DL && unset DVIM_SYS && unset DVIM_VER
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
