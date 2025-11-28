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
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
