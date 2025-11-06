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
