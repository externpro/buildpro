ARG BPROTAG=latest
FROM ghcr.io/externpro/buildpro/rocky-pro:${BPROTAG}
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source=https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# target architecture and OS
ARG TARGETARCH # e.g., amd64, arm64
ARG TARGETOS # e.g., linux
RUN echo "Building for architecture ${TARGETARCH} on OS ${TARGETOS}"
# https://rockylinux.pkgs.org https://rhel.pkgs.org
# AppStream, BaseOS Repositories
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     epel-release \
     iproute \
     libSM-devel \
     postgresql-devel \
     rpm-build \
     rpm-sign \
     Xvfb \
  && ${DNF} clean all
# PowerTools Repository
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=powertools ${DNFOPT} \
     cppcheck \
     xeyes \
  && ${DNF} clean all
# EPEL Repository
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=epel ${DNFOPT} \
     gperftools \
  && ${DNF} clean all
# lcov deps
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=powertools ${DNFOPT} \
     perl-IO-Compress \
     perl-JSON-XS \
     perl-Module-Load-Conditional \
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
# doxygen
RUN export DXY_VER=1.8.13 \
  && wget -qO- --no-check-certificate \
  "https://downloads.sourceforge.net/project/doxygen/rel-${DXY_VER}/doxygen-${DXY_VER}.linux.bin.tar.gz" \
  | tar --no-same-owner -xz -C /usr/local/ \
  && mv /usr/local/doxygen-${DXY_VER}/bin/doxygen /usr/local/bin/ \
  && rm -rf /usr/local/doxygen-${DXY_VER}/ \
  && unset DXY_VER
# dotnet
RUN rpm -Uvh https://packages.microsoft.com/config/rocky/8/packages-microsoft-prod.rpm \
  && ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     dotnet-sdk-8.0 \
  && ${DNF} clean all
ENV DOTNET_CLI_TELEMETRY_OPTOUT=true
# minimum chrome
RUN export CHR_VER=133.0.6943.98 \
  && export CHR_DL=linux/chrome/rpm/stable/$(uname -m)/google-chrome-stable-${CHR_VER}-1.$(uname -m).rpm \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     https://dl.google.com/${CHR_DL} \
  && ${DNF} clean all \
  && unset CHR_DL && unset CHR_VER
# externpro
ENV XP_VER=24.05
ENV EXTERNPRO_PATH=${EXTERN_DIR}/externpro-${XP_VER}-${GCC_VER}-64-Linux
RUN mkdir ${EXTERN_DIR} \
  && export XP_DL=releases/download/24.09/externpro-${XP_VER}-${GCC_VER}-64-$(uname -s).tar.xz \
  && wget -qO- "https://github.com/externpro/externpro/${XP_DL}" | tar --no-same-owner -xJ -C ${EXTERN_DIR} \
  && unset XP_DL
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
