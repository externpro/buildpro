FROM ghcr.io/smanders/buildpro/rocky9-pro:latest
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# https://rockylinux.pkgs.org https://rhel.pkgs.org
# AppStream, BaseOS Repositories
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     ghostscript `#LaTeX` \
     graphviz \
     iproute \
     libSM-devel \
     rpm-build \
     Xvfb \
  && dnf clean all
# PowerTools, EPEL Repositories
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     dnf-plugins-core \
     epel-release \
  && dnf config-manager --set-enabled devel \
  && dnf -y update \
  && dnf -y install --setopt=tsflags=nodocs \
     cppcheck \
     gperftools \
     unixODBC-devel \
  && dnf clean all
# lcov (and LaTeX?) deps
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     perl-Digest-MD5 \
     perl-File-Copy \
     perl-IO-Compress \
     perl-JSON-XS \
     perl-Module-Load-Conditional \
  && dnf clean all
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
  && wget -qO- "https://github.com/git-lfs/git-lfs/releases/download/v${LFS_VER}/git-lfs-linux-amd64-v${LFS_VER}.tar.gz" \
  | tar -xz -C /usr/local/src/lfs \
  && /usr/local/src/lfs/install.sh \
  && rm -rf /usr/local/src/lfs/ \
  && unset LFS_VER \
  && git lfs install --system
# doxygen
RUN export DXY_VER=1.8.13 \
  && wget -qO- --no-check-certificate \
  "https://downloads.sourceforge.net/project/doxygen/rel-${DXY_VER}/doxygen-${DXY_VER}.linux.bin.tar.gz" \
  | tar -xz -C /usr/local/ \
  && mv /usr/local/doxygen-${DXY_VER}/bin/doxygen /usr/local/bin/ \
  && rm -rf /usr/local/doxygen-${DXY_VER}/ \
  && unset DXY_VER
# LaTeX
# NOTE: multiple layers, small subset of collection-latexextra to reduce layer sizes
COPY texlive.profile /usr/local/src/
RUN export TEX_VER=2017 \
  && wget -qO- "http://ftp.math.utah.edu/pub/tex/historic/systems/texlive/${TEX_VER}/tlnet-final/install-tl-unx.tar.gz" \
  | tar -xz -C /usr/local/src/ \
  && /usr/local/src/install-tl-20180303/install-tl -profile /usr/local/src/texlive.profile \
     -repository http://ftp.math.utah.edu/pub/tex/historic/systems/texlive/${TEX_VER}/tlnet-final/archive/ \
  && rm -rf /usr/local/src/install-tl-20180303 /usr/local/src/texlive.profile \
  && unset TEX_VER
RUN  tlmgr install collection-fontsrecommended \
  && tlmgr install collection-latexrecommended \
  && tlmgr install tabu varwidth multirow wrapfig adjustbox collectbox sectsty tocloft `#collection-latexextra` \
  && tlmgr install epstopdf
ENV PATH=$PATH:/usr/local/texlive/2017/bin/x86_64-linux
# CUDA https://developer.nvidia.com/cuda-11-7-1-download-archive
# NOTE: only subset of cuda-libraries-devel to reduce layer sizes
RUN export CUDA_VER=11-7 \
  && export CUDA_DL=https://developer.download.nvidia.com/compute/cuda/repos/rhel7/$(uname -m) \
  && dnf config-manager --add-repo ${CUDA_DL}/cuda-rhel7.repo \
  && dnf clean all \
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA ${CUDA_DL}/D42D0685.pub \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA \
  && dnf -y install \
     cuda-compiler-${CUDA_VER} \
     cuda-cudart-devel-${CUDA_VER} \
  `# cuda-libraries-devel` \
     libcublas-devel-${CUDA_VER} \
     libcufft-devel-${CUDA_VER} \
     libcusolver-devel-${CUDA_VER} \
     libcusparse-devel-${CUDA_VER} \
  && dnf clean all \
  && unset CUDA_DL && unset CUDA_VER
ENV PATH=$PATH:/usr/local/cuda/bin
# dotnet
RUN rpm -Uvh https://packages.microsoft.com/config/centos/7/packages-microsoft-prod.rpm \
  && dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     dotnet-sdk-3.1 \
  && dnf clean all
ENV DOTNET_CLI_TELEMETRY_OPTOUT=true
# minimum chrome
RUN export CHR_VER=108.0.5359.98 \
  && export CHR_DL=linux/chrome/rpm/stable/$(uname -m)/google-chrome-stable-${CHR_VER}-1.$(uname -m).rpm \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     https://dl.google.com/${CHR_DL} \
  && dnf clean all \
  && unset CHR_DL && unset CHR_VER
# minimum firefox
RUN export FOX_VER=102.6.0esr \
  && export FOX_DL=pub/firefox/releases/${FOX_VER}/linux-$(uname -m)/en-US/firefox-${FOX_VER}.tar.bz2 \
  && wget -qO- "https://ftp.mozilla.org/${FOX_DL}" | tar -xj -C /opt/ \
  && ln -s /opt/firefox/firefox /usr/local/bin/firefox \
  && unset FOX_DL && unset FOX_VER
# externpro
ENV XP_VER=23.02
ENV EXTERNPRO_PATH=${EXTERN_DIR}/externpro-${XP_VER}-${GCC_VER}-64-Linux
RUN mkdir ${EXTERN_DIR} \
  && export XP_DL=releases/download/${XP_VER}/externpro-${XP_VER}-${GCC_VER}-64-$(uname -s).tar.xz \
# && wget -qO- "https://github.com/smanders/externpro/${XP_DL}" | tar -xJ -C ${EXTERN_DIR} \
  && unset XP_DL
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
