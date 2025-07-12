ARG BPROTAG=latest
FROM ghcr.io/externpro/buildpro/rocky-mdv:${BPROTAG}
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source=https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# target architecture and OS
ARG TARGETARCH # e.g., amd64, arm64
ARG TARGETOS # e.g., linux
RUN echo "Building for architecture ${TARGETARCH} on OS ${TARGETOS}"
# install node
ENV NODE_VERSION=20.18.0
ENV NVM_DIR=/opt/.nvm
RUN mkdir $NVM_DIR && ${DNF} install -y curl \
  && curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.1/install.sh | bash \
  && ${DNF} clean all
RUN . "$NVM_DIR/nvm.sh" && nvm install ${NODE_VERSION} \
  && . "$NVM_DIR/nvm.sh" && nvm use v${NODE_VERSION} \
  && . "$NVM_DIR/nvm.sh" && nvm alias default v${NODE_VERSION}
ENV PATH="$NVM_DIR/versions/node/v${NODE_VERSION}/bin/:${PATH}"
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
