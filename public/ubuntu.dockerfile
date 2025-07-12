FROM ubuntu:20.04
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source=https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# target architecture and OS
ARG TARGETARCH # e.g., amd64, arm64
ARG TARGETOS # e.g., linux
RUN echo "Building for architecture ${TARGETARCH} on OS ${TARGETOS}"
# apt repositories
RUN apt update \
  && DEBIAN_FRONTEND=noninteractive \
  apt -y --quiet --no-install-recommends install \
     ca-certificates \
     lsb-release \
     software-properties-common \
     sudo \
     tzdata \
  && apt -y autoremove \
  && apt clean autoclean \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
RUN add-apt-repository ppa:git-core/ppa -y \
  && apt update \
  && DEBIAN_FRONTEND=noninteractive \
  apt -y --quiet --no-install-recommends install \
     git \
     ninja-build \
     openssh-client \
     vim \
     wget \
     xz-utils \
  && apt -y autoremove \
  && apt clean autoclean \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
# Bootlin Toolchain gcc 9.3
RUN mkdir -p /opt/jetson/jetpack5-gcc \
  && wget -qO- "https://developer.nvidia.com/embedded/jetson-linux/bootlin-toolchain-gcc-93" \
  | tar -xz -C /opt/jetson/jetpack5-gcc
ENV XPX=JetPack
# git-lfs
RUN export LFS_VER=2.12.1 \
  && mkdir /usr/local/src/lfs \
  && wget -qO- "https://github.com/git-lfs/git-lfs/releases/download/v${LFS_VER}/git-lfs-linux-amd64-v${LFS_VER}.tar.gz" \
  | tar -xz -C /usr/local/src/lfs \
  && /usr/local/src/lfs/install.sh \
  && rm -rf /usr/local/src/lfs/ \
  && unset LFS_VER \
  && git lfs install --system
# cmake
RUN export CMK_VER=3.31.6 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-$(uname -s)-$(uname -m).tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# Dockerfile.vim
RUN export DVIM_VER=21.09.06 \
  && export DVIM_SYS=/usr/share/vim/vimfiles \
  && export DVIM_DL=releases/download/${DVIM_VER}/Dockerfile.vim-${DVIM_VER}.tar.xz \
  && mkdir -p ${DVIM_SYS} \
  && wget -qO- "https://github.com/smanders/Dockerfile.vim/${DVIM_DL}" | tar --no-same-owner -xJ -C ${DVIM_SYS} \
  && unset DVIM_DL && unset DVIM_SYS && unset DVIM_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /usr/local/bpbin/
# source git-prompt.sh
RUN echo "[ -f /usr/local/bpbin/git-prompt.sh ] && source /usr/local/bpbin/git-prompt.sh" \
  >> /etc/skel/.bashrc
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
