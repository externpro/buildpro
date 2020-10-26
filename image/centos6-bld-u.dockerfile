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
RUN export CRT_VER=20.10.1 && export CRW_VER=20.07.1 \
  && mkdir /opt/extern/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/${CRW_VER}/CRTool-${CRW_VER}.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/${CRT_VER}/CRToolImpl-${CRT_VER}.sh" \
  && chmod 755 CRTool*.sh \
  && ./CRTool-${CRW_VER}.sh --prefix=/opt/extern/CRTool --exclude-subdir \
  && ./CRToolImpl-${CRT_VER}.sh --prefix=/opt/extern --include-subdir \
  && rm CRTool-${CRW_VER}.sh \
  && rm CRToolImpl-${CRT_VER}.sh \
  && unset CRT_VER && unset CRW_VER
ENV PATH=$PATH:/opt/extern/CRTool
# SDLPluginSDK
RUN export SDK_VER=v3.2.0.0 \
  && export SDK_DL=releases/download/${SDK_VER}/SDLPluginSDK-${SDK_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/PluginFramework/SDKSuper/${SDK_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset SDK_DL && unset SDK_VER
# internpro
RUN export IP_VER=20.10.1 \
  && export IP_DL=releases/download/${IP_VER}/internpro-${IP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/${IP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset IP_DL && unset IP_VER
# webpro
RUN export WP_VER=20.10.3 \
  && export WP_DL=releases/download/${WP_VER}/webpro-${WP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset WP_DL && unset WP_VER
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
