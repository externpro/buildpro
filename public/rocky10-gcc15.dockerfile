FROM rockylinux/rockylinux:10
ENV GTS=gcc-toolset-15
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
     gdb \
     git \
     git-lfs \
     graphviz \
     gtk3-devel \
     iproute \
     libSM-devel \
     mesa-libGL-devel \
     mesa-libGLU-devel \
     perf \
     postgresql-devel \
     python3-devel \
     rpm-build \
     rpm-sign \
     sudo \
     vim \
     wget \
     xorg-x11-xauth \
     xz \
  && ${DNF} clean all \
  && git lfs install --system \
  && rm -rf /var/cache/dnf
# gcc-toolset (aka GTS)
RUN dnf_retry update \
  && dnf_retry install ${DNFOPT} \
     ${GTS}-binutils \
     ${GTS}-gcc \
     ${GTS}-gcc-c++ \
     ${GTS}-libasan-devel \
     ${GTS}-libtsan-devel \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# CRB (Code Ready Builder) Repository
RUN dnf_retry update \
  && dnf_retry install --enablerepo=crb ${DNFOPT} \
     cppcheck \
     ninja-build \
     perl-Capture-Tiny `#lcov` \
     perl-DateTime `#lcov` \
     perl-JSON-XS `#lcov` \
     perl-Module-Load-Conditional `#lcov` \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# EPEL Repository
RUN dnf_retry update \
  && dnf_retry install --enablerepo=epel ${DNFOPT} \
     bat \
     gperftools \
     lcov \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# AlmaLinux Devel Repository
RUN dnf_retry install \
  --repofrompath=alma10-devel,'https://repo.almalinux.org/almalinux/10/devel/$basearch/os/' \
  --enablerepo=alma10-devel --nogpgcheck ${DNFOPT} \
    xorg-x11-server-Xvfb \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
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
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-$(uname -s)-$(uname -m).tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
# environment: gcc-toolset, enable scl binaries
ENV PATH="/opt/rh/${GTS}/root/usr/bin:${PATH}" \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
