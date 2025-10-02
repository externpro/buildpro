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
# exdlpro
ENV XP_VER=25.04
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
