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
# doxygen
RUN export DXY_VER=1.8.13 \
  && wget -qO- --no-check-certificate \
  "https://downloads.sourceforge.net/project/doxygen/rel-${DXY_VER}/doxygen-${DXY_VER}.linux.bin.tar.gz" \
  | tar --no-same-owner -xz -C /usr/local/ \
  && mv /usr/local/doxygen-${DXY_VER}/bin/doxygen /usr/local/bin/ \
  && rm -rf /usr/local/doxygen-${DXY_VER}/ \
  && unset DXY_VER
# CUDA https://developer.nvidia.com/cuda-toolkit-archive
RUN export CUDA_VER=12-6 \
  && export CUDA_DL=https://developer.download.nvidia.com/compute/cuda/repos/rhel8/$(uname -m) \
  `# microdnf config-manager --add-repo ${CUDA_DL}/cuda-rhel8.repo : error config-manager` \
  `# so use wget and put .repo file in /etc/yum.repos.d manually...` \
  && wget -O /etc/yum.repos.d/cuda-rhel8.repo ${CUDA_DL}/cuda-rhel8.repo \
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA ${CUDA_DL}/D42D0685.pub \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-NVIDIA \
  && ${DNF} -y update \
  && ${DNF} clean all \
  && ${DNF} -y install \
     cuda-toolkit-${CUDA_VER} \
  && ${DNF} clean all \
  && unset CUDA_DL && unset CUDA_VER
RUN ${DNF} clean all \
  && ${DNF} -y install \
  `# https://developer.nvidia.com/cudnn` \
     cudnn \
  `# https://developer.nvidia.com/cudss` \
     cudss \
  `# https://developer.nvidia.com/cutensor` \
     libcutensor2 \
     libcutensor-devel \
     libcutensor-doc \
  && ${DNF} clean all
ENV PATH=$PATH:/usr/local/cuda/bin
# exdlpro
ENV XP_VER=25.06
RUN mkdir -p ${EXTERN_DIR} \
  && OS="$(uname -s)" \
  && ARCH="$(uname -m)" \
  && if [ "$ARCH" = "aarch64" ]; then \
       PKG="${OS}-arm64-devel"; \
     else \
       PKG="${OS}-devel"; \
     fi \
  && echo "Detected OS: $OS" \
  && echo "Detected ARCH: $ARCH" \
  && echo "PKG=${PKG}" \
  && export XP_DL=releases/download/v${XP_VER}/exdlpro-v${XP_VER}-${GCC_VER}-64-${PKG}.tar.xz \
  && wget -qO- "https://github.com/externpro/exdlpro/${XP_DL}" | tar --no-same-owner -xJ -C ${EXTERN_DIR} \
  && unset XP_DL && unset PKG
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
