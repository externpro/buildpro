FROM rockylinux:8.9
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source=https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# target architecture and OS
ARG TARGETARCH # e.g., amd64, arm64
ARG TARGETOS # e.g., linux
RUN echo "Building for architecture ${TARGETARCH} on OS ${TARGETOS}"
# dnf/microdnf (see https://github.com/externpro/buildpro/issues/107#issuecomment-2770817750)
ENV DNF=dnf
ENV DNFOPT="--setopt=tsflags=nodocs --setopt=install_weak_deps=0"
# initial dnf update
RUN ${DNF} -y update \
  && ${DNF} clean all
# dnf repositories
# https://rockylinux.pkgs.org https://rhel.pkgs.org
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     bat \
     coreutils-common \
     epel-release \
     git \
     graphviz \
     gtk3-devel \
     iproute \
     libSM-devel \
     mesa-libGL-devel \
     mesa-libGLU-devel \
     perf \
     postgresql-devel \
     python39-devel \
     redhat-lsb-core \
     rpm-build \
     rpm-sign \
     sudo \
     vim \
     wget \
     xorg-x11-utils \
     xorg-x11-xauth \
     Xvfb \
     xz \
  && ${DNF} clean all \
  && alternatives --set python3 $(command -v python3.9)
# gcc-toolset
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     gcc-toolset-9-binutils \
     gcc-toolset-9-gcc \
     gcc-toolset-9-gcc-c++ \
     gcc-toolset-9-gdb \
     gcc-toolset-9-libasan-devel \
     gcc-toolset-9-libtsan-devel \
     gcc-toolset-9-make \
  && ${DNF} clean all
# PowerTools Repository
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=powertools ${DNFOPT} \
     cppcheck \
     perl-IO-Compress `#lcov` \
     perl-JSON-XS `#lcov` \
     perl-Module-Load-Conditional `#lcov` \
     xeyes \
  && ${DNF} clean all
# EPEL Repository
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=epel ${DNFOPT} \
     gperftools \
  && ${DNF} clean all
# lcov
RUN export LCOV_VER=1.16 \
  && wget -qO- "https://github.com/linux-test-project/lcov/releases/download/v${LCOV_VER}/lcov-${LCOV_VER}.tar.gz" \
  | tar -xz -C /usr/local/src \
  && (cd /usr/local/src/lcov-${LCOV_VER} && make install > /dev/null) \
  && rm -rf /usr/local/src/lcov-${LCOV_VER} \
  && unset LCOV_VER
# git-lfs
RUN export LFS_VER=2.12.1 \
  && mkdir /usr/local/src/lfs \
  && wget -qO- "https://github.com/git-lfs/git-lfs/releases/download/v${LFS_VER}/git-lfs-${TARGETOS}-${TARGETARCH}-v${LFS_VER}.tar.gz" \
  | tar -xz -C /usr/local/src/lfs \
  && /usr/local/src/lfs/install.sh \
  && rm -rf /usr/local/src/lfs/ \
  && unset LFS_VER \
  && git lfs install --system
# Dockerfile.vim
RUN export DVIM_VER=21.09.06 \
  && export DVIM_SYS=/usr/share/vim/vimfiles \
  && export DVIM_DL=releases/download/${DVIM_VER}/Dockerfile.vim-${DVIM_VER}.tar.xz \
  && wget -qO- "https://github.com/smanders/Dockerfile.vim/${DVIM_DL}" | tar --no-same-owner -xJ -C ${DVIM_SYS} \
  && unset DVIM_DL && unset DVIM_SYS && unset DVIM_VER
# ninja
RUN export NJA_VER=1.13.1 \
  && export NJA_DL=ninja-linux$([ "$(uname -m)" = "aarch64" ] && echo "-$(uname -m)").zip \
  && wget -q "https://github.com/ninja-build/ninja/releases/download/v${NJA_VER}/${NJA_DL}" -P /usr/local/src \
  && unzip /usr/local/src/${NJA_DL} -d /usr/local/bin/ \
  && rm /usr/local/src/${NJA_DL} \
  && unset NJA_DL && unset NJA_VER
# cmake
RUN export CMK_VER=3.31.6 \
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
