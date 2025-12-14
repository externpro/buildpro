FROM rockylinux/rockylinux:8
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
ARG ROCKY_BASEURL_PREFIX="https://dl.rockylinux.org"
RUN shopt -s nullglob \
  && repo_files=(/etc/yum.repos.d/*.repo) \
  && [ "${#repo_files[@]}" -gt 0 ] \
  && tail='/$contentdir/$releasever' \
  && case "${ROCKY_BASEURL_PREFIX}" in *"/vault/rocky/"*) tail='' ;; esac \
  && sed -i \
     -e 's/^mirrorlist=/#mirrorlist=/g' \
     -e 's|^#baseurl=http://dl\.rockylinux\.org/\$contentdir/\$releasever|baseurl='"${ROCKY_BASEURL_PREFIX}${tail}"'|g' \
     -e 's|^#baseurl=https://dl\.rockylinux\.org/\$contentdir/\$releasever|baseurl='"${ROCKY_BASEURL_PREFIX}${tail}"'|g' \
     -e 's|^#baseurl=http://download\.rockylinux\.org/\$contentdir/\$releasever|baseurl='"${ROCKY_BASEURL_PREFIX}${tail}"'|g' \
     -e 's|^#baseurl=https://download\.rockylinux\.org/\$contentdir/\$releasever|baseurl='"${ROCKY_BASEURL_PREFIX}${tail}"'|g' \
     "${repo_files[@]}"
RUN cat > /usr/local/bin/dnf_retry <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
dnf_bin="${DNF:-dnf}"
for i in 1 2 3 4 5; do
  "${dnf_bin}" clean all || true
  rm -rf /var/cache/dnf
  if "${dnf_bin}" -y "$@"; then
    exit 0
  fi
  if [ "${i}" -eq 5 ]; then
    exit 1
  fi
  sleep $((i * 5))
done
EOF
RUN chmod +x /usr/local/bin/dnf_retry
# initial dnf update
RUN dnf_retry update
# dnf repositories
# https://rockylinux.pkgs.org https://rhel.pkgs.org
RUN dnf_retry update \
  && dnf_retry install ${DNFOPT} \
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
  && rm -rf /var/cache/dnf \
  && alternatives --set python3 $(command -v python3.9)
# gcc-toolset
RUN dnf_retry update \
  && dnf_retry install ${DNFOPT} \
     gcc-toolset-9-binutils \
     gcc-toolset-9-gcc \
     gcc-toolset-9-gcc-c++ \
     gcc-toolset-9-gdb \
     gcc-toolset-9-libasan-devel \
     gcc-toolset-9-libtsan-devel \
     gcc-toolset-9-make \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# PowerTools Repository
RUN dnf_retry update \
  && dnf_retry install --enablerepo=powertools ${DNFOPT} \
     cppcheck \
     perl-Capture-Tiny `#lcov` \
     perl-DateTime `#lcov` \
     perl-IO-Compress `#lcov` \
     perl-JSON-XS `#lcov` \
     perl-Module-Load-Conditional `#lcov` \
     perl-Time-HiRes `#lcov` \
     xeyes \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# EPEL Repository
RUN dnf_retry update \
  && dnf_retry install --enablerepo=epel ${DNFOPT} \
     bat \
     gperftools \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# lcov
RUN export LCOV_VER=2.3.2 \
  && rm -rf /usr/local/src/lcov \
  && git clone -q --depth 1 --branch "v${LCOV_VER}" https://github.com/linux-test-project/lcov.git /usr/local/src/lcov \
  && (cd /usr/local/src/lcov && make install > /dev/null) \
  && rm -rf /usr/local/src/lcov \
  && unset LCOV_VER
# git-lfs
RUN export LFS_VER=3.7.1 \
  && mkdir /usr/local/src/lfs \
  && wget -qO- "https://github.com/git-lfs/git-lfs/releases/download/v${LFS_VER}/git-lfs-${TARGETOS}-${TARGETARCH}-v${LFS_VER}.tar.gz" \
  | tar -xz -C /usr/local/src/lfs \
  && /usr/local/src/lfs/git-lfs-${LFS_VER}/install.sh \
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
RUN export NJA_VER=1.13.2 \
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
# environment: gcc-toolset, enable scl binaries
ENV PATH="/opt/rh/gcc-toolset-9/root/usr/bin:${PATH}" \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
