FROM ubuntu:24.04
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
     bat `#batcat` \
     build-essential \
     git \
     git-lfs \
     less \
     libgl1-mesa-dev \
     libglu1-mesa-dev \
     libgtk-3-dev \
     ninja-build \
     openssh-client \
     python3-dev \
     vim \
     wget \
     xvfb \
     xz-utils \
  && apt -y autoremove \
  && apt clean autoclean \
  && git lfs install --system \
  && rm -rf /var/lib/apt/lists/{apt,dpkg,cache,log} /tmp/* /var/tmp/*
# Dockerfile.vim
RUN export DVIM_VER=21.09.06.1 \
  && export DVIM_SYS=/usr/share/vim/vimfiles \
  && export DVIM_SH=Dockerfile.vim-v${DVIM_VER}.sh \
  && export DVIM_DL=releases/download/v${DVIM_VER}/${DVIM_SH} \
  && mkdir -p ${DVIM_SYS} \
  && wget -q "https://github.com/smanders/Dockerfile.vim/${DVIM_DL}" \
  && sh ./${DVIM_SH} --skip-license --prefix=${DVIM_SYS} \
  && rm -f ./${DVIM_SH} \
  && unset DVIM_DL && unset DVIM_SH && unset DVIM_SYS && unset DVIM_VER
# cmake
RUN export CMK_VER=3.31.6 \
  && export CMK_SH=cmake-${CMK_VER}-$(uname -s)-$(uname -m).sh \
  && export CMK_DL=releases/download/v${CMK_VER}/${CMK_SH} \
  && wget -q "https://github.com/Kitware/CMake/${CMK_DL}" \
  && sh ./${CMK_SH} --skip-license --prefix=/usr/local/ \
  && rm -f ./${CMK_SH} \
  && unset CMK_DL && unset CMK_SH && unset CMK_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /usr/local/bpbin/
# source git-prompt.sh
RUN echo "[ -f /usr/local/bpbin/git-prompt.sh ] && source /usr/local/bpbin/git-prompt.sh" \
  >> /etc/skel/.bashrc
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
