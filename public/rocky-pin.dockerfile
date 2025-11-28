ARG BPROTAG=latest
FROM ghcr.io/externpro/buildpro/rocky8-gcc9:${BPROTAG}
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source=https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# target architecture and OS
ARG TARGETARCH # e.g., amd64, arm64
ARG TARGETOS # e.g., linux
RUN echo "Building for architecture ${TARGETARCH} on OS ${TARGETOS}"
# LaTeX deps
RUN ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install --enablerepo=powertools ${DNFOPT} \
     ghostscript `#LaTeX` \
     perl-Digest-MD5 `#LaTeX` \
  && ${DNF} clean all
# doxygen
RUN export DXY_VER=1.8.13 \
  && wget -qO- --no-check-certificate \
  "https://downloads.sourceforge.net/project/doxygen/rel-${DXY_VER}/doxygen-${DXY_VER}.linux.bin.tar.gz" \
  | tar --no-same-owner -xz -C /usr/local/ \
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
# CUDA https://developer.nvidia.com/cuda-toolkit-archive
# NOTE: only subset of cuda-libraries-devel to reduce layer sizes
RUN export CUDA_VER=12-6 \
  && export CUDA_DL=https://developer.download.nvidia.com/compute/cuda/repos/rhel8/$(uname -m) \
  `# microdnf config-manager --add-repo ${CUDA_DL}/cuda-rhel8.repo : error config-manager` \
  `# so use wget and put .repo file in /etc/yum.repos.d manually...` \
  && wget -O /etc/yum.repos.d/cuda-rhel8.repo ${CUDA_DL}/cuda-rhel8.repo \
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA ${CUDA_DL}/D42D0685.pub \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA \
  && ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install ${DNFOPT} \
     cuda-compiler-${CUDA_VER} \
     cuda-cudart-devel-${CUDA_VER} \
  `# cuda-libraries-devel` \
     libcublas-devel-${CUDA_VER} \
     libcufft-devel-${CUDA_VER} \
     libcusolver-devel-${CUDA_VER} \
     libcusparse-devel-${CUDA_VER} \
  && ${DNF} clean all \
  && unset CUDA_DL && unset CUDA_VER
ENV PATH=$PATH:/usr/local/cuda/bin
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
