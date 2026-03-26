FROM rockylinux/rockylinux:8
ENV GTS=gcc-toolset-9
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
     git-lfs \
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
  && git lfs install --system \
  && alternatives --set python3 $(command -v python3.9) \
  && rm -rf /var/cache/dnf
# gcc-toolset (aka GTS)
RUN dnf_retry update \
  && dnf_retry install ${DNFOPT} \
     ${GTS}-binutils \
     ${GTS}-gcc \
     ${GTS}-gcc-c++ \
     ${GTS}-gdb \
     ${GTS}-libasan-devel \
     ${GTS}-libtsan-devel \
     ${GTS}-make \
  && ${DNF} clean all \
  && rm -rf /var/cache/dnf
# PowerTools Repository
RUN dnf_retry update \
  && dnf_retry install --enablerepo=powertools ${DNFOPT} \
     cppcheck \
     perl-Capture-Tiny `#lcov` \
     perl-DateTime `#lcov` \
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
RUN export LCOV_VER=2.0 && export LCOV_REL=1 \
  && export LCOV_RPM=lcov-${LCOV_VER}-${LCOV_REL}.noarch.rpm \
  && wget -q "https://github.com/linux-test-project/lcov/releases/download/v${LCOV_VER}/${LCOV_RPM}" \
  && dnf_retry install ./${LCOV_RPM} \
  && rm -f ./${LCOV_RPM} \
  && unset LCOV_RPM && unset LCOV_REL && unset LCOV_VER
# Dockerfile.vim
RUN export DVIM_VER=21.09.06.4 \
  && export DVIM_RPM=Dockerfile.vim-xpv${DVIM_VER}.rpm \
  && wget -q "https://github.com/smanders/Dockerfile.vim/releases/download/xpv${DVIM_VER}/${DVIM_RPM}" \
  && dnf_retry install ./${DVIM_RPM} \
  && rm -f ./${DVIM_RPM} \
  && unset DVIM_RPM && unset DVIM_VER
# ninja
RUN export NJA_VER=1.13.2 \
  && export NJA_DL=ninja-linux$([ "$(uname -m)" = "aarch64" ] && echo "-$(uname -m)").zip \
  && wget -q "https://github.com/ninja-build/ninja/releases/download/v${NJA_VER}/${NJA_DL}" -P /usr/local/src \
  && unzip /usr/local/src/${NJA_DL} -d /usr/local/bin/ \
  && rm /usr/local/src/${NJA_DL} \
  && unset NJA_DL && unset NJA_VER
# cmake
RUN export CMK_VER=4.3.0.1 \
  && export CMK_RPM=cmake-pkg-xpv${CMK_VER}-$(uname -s)-$(uname -m).rpm \
  && wget -q "https://github.com/externpro/cmake-pkg/releases/download/xpv${CMK_VER}/${CMK_RPM}" \
  && dnf_retry install ./${CMK_RPM} \
  && rm -f ./${CMK_RPM} \
  && unset CMK_RPM && unset CMK_VER
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
# environment: gcc-toolset, enable scl binaries
ENV PATH="/opt/rh/${GTS}/root/usr/bin:${PATH}" \
    BASH_ENV="/usr/local/bpbin/scl_enable" \
    ENV="/usr/local/bpbin/scl_enable" \
    PROMPT_COMMAND=". /usr/local/bpbin/scl_enable"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
