FROM ghcr.io/smanders/buildpro/centos7-pro:22.04.1
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# yum repositories
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     ghostscript `#LaTeX` \
     gperftools \
     graphviz \
     iproute \
     libSM-devel.x86_64 \
     rpm-build \
     unixODBC-devel \
     xeyes \
     https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm \
  && yum clean all
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     cppcheck `#epel` \
     lcov `#epel` \
  && yum clean all
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
# CUDA https://developer.nvidia.com/cuda-10.1-download-archive-update1
# NOTE: only subset of cuda-libraries-dev to reduce layer sizes
RUN export CUDA_VER=10.1.168-1 \
  && export CUDA_RPM=cuda-repo-rhel7-${CUDA_VER}.x86_64.rpm \
  && wget -q "https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/${CUDA_RPM}" \
  && rpm --install ${CUDA_RPM} \
  && yum clean all \
  # https://forums.developer.nvidia.com/t/updating-the-cuda-linux-gpg-repository-key/212897/10
  # https://github.com/NVIDIA/cuda-repo-management/issues/4
  && yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/cuda-rhel7.repo \
  # https://codingbee.net/uncategorized/yum-error-public-key-for-rpm-is-not-installed
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA http://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/D42D0685.pub \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA \
  && yum -y install \
     cuda-compiler-10-1 \
  `# cuda-libraries-dev-10-1` \
     cuda-cudart-dev-10-1 \
     cuda-cufft-dev-10-1 \
     cuda-cusolver-dev-10-1 \
     cuda-cusparse-dev-10-1 \
     libcublas-devel-10-2 \
  && ln -s cuda-10.1 /usr/local/cuda \
  && yum clean all \
  && rm ${CUDA_RPM} \
  `# libcublas installed to 10.2, move to 10.1` \
  && mv /usr/local/cuda-10.2/targets/x86_64-linux/include/* /usr/local/cuda-10.1/targets/x86_64-linux/include/ \
  && mv /usr/local/cuda-10.2/targets/x86_64-linux/lib/stubs/* /usr/local/cuda-10.1/targets/x86_64-linux/lib/stubs/ \
  && rmdir /usr/local/cuda-10.2/targets/x86_64-linux/lib/stubs/ \
  && mv /usr/local/cuda-10.2/targets/x86_64-linux/lib/* /usr/local/cuda-10.1/targets/x86_64-linux/lib/ \
  && rm -rf /usr/local/cuda-10.2 \
  && unset CUDA_VER && unset CUDA_RPM
ENV PATH=$PATH:/usr/local/cuda/bin
# dotnet
RUN rpm -Uvh https://packages.microsoft.com/config/centos/7/packages-microsoft-prod.rpm \
  && yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     dotnet-sdk-3.1 \
  && yum clean all
ENV DOTNET_CLI_TELEMETRY_OPTOUT=true
# chrome
RUN export CHR_VER=94.0.4606.81 \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     https://dl.google.com/linux/chrome/rpm/stable/x86_64/google-chrome-stable-${CHR_VER}-1.x86_64.rpm \
  && yum clean all \
  && unset CHR_VER
# externpro
ENV XP_VER=22.03
ENV EXTERNPRO_PATH=${EXTERN_DIR}/externpro-${XP_VER}-${GCC_VER}-64-Linux
RUN mkdir ${EXTERN_DIR} \
  && export XP_DL=releases/download/${XP_VER}/externpro-${XP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://github.com/smanders/externpro/${XP_DL}" \
   | tar -xJ -C ${EXTERN_DIR} \
  && unset XP_DL
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
