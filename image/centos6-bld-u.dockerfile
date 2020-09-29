FROM ghcr.io/smanders/buildpro/centos6-bld:latest
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /scripts
VOLUME /srcdir
# enable scl binaries
ENV BASH_ENV="/scripts/scl_enable" \
    ENV="/scripts/scl_enable" \
    PROMPT_COMMAND=". /scripts/scl_enable"
# CRTool
RUN mkdir /opt/extern/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/20.07.1/CRTool-20.07.1.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/20.05.2/CRToolImpl-20.05.2.sh" \
  && chmod 755 CRTool*.sh \
  && ./CRTool-20.07.1.sh --prefix=/opt/extern/CRTool --exclude-subdir \
  && ./CRToolImpl-20.05.2.sh --prefix=/opt/extern --include-subdir \
  && rm CRTool-20.07.1.sh \
  && rm CRToolImpl-20.05.2.sh
ENV PATH=$PATH:/opt/extern/CRTool
# SDLPluginSDK
RUN export SDK_VER=v3.0.3.0 \
  && export SDK_DL=releases/download/${SDK_VER}/VantagePluginSDK-${SDK_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/PluginFramework/SDKSuper/${SDK_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset SDK_DL && unset SDK_VER
# internpro
RUN export IP_VER=20.01.3 \
  && export IP_DL=releases/download/${IP_VER}/internpro-${IP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/${IP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset IP_DL && unset IP_VER
# create non-root user, add to sudoers
ARG USERNAME
ARG USERID
RUN adduser --uid ${USERID} ${USERNAME} \
  && echo "" >> /etc/sudoers \
  && echo "## dockerfile adds ${USERNAME} to sudoers" >> /etc/sudoers \
  && echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
ENV USER=${USERNAME}
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/scripts/entry.sh"]
