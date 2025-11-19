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
ENV PATH=$PATH:/usr/local/cuda/bin
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
