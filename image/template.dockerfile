FROM ghcr.io/smanders/buildpro/%%BP_REPO%%:%%BP_TAG%%
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# CRTool
ARG CRT_VER
ARG CRW_VER
RUN if [[ -n "${CRW_VER}" && -n "${CRT_VER}" ]]; then \
  mkdir /opt/extern/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/${CRW_VER}/CRTool-${CRW_VER}.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/${CRT_VER}/CRToolImpl-${CRT_VER}.sh" \
  && chmod 755 CRTool*.sh \
  && ./CRTool-${CRW_VER}.sh --prefix=/opt/extern/CRTool --exclude-subdir \
  && ./CRToolImpl-${CRT_VER}.sh --prefix=/opt/extern --include-subdir \
  && rm CRTool-${CRW_VER}.sh \
  && rm CRToolImpl-${CRT_VER}.sh \
  ; fi
ENV PATH=$PATH:/opt/extern/CRTool
# SDLPluginSDK
ARG SDK_VER
RUN if [[ -n "${SDK_VER}" ]]; then \
  export SDK_DL=releases/download/${SDK_VER}/SDLPluginSDK-${SDK_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/PluginFramework/SDKSuper/${SDK_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset SDK_DL \
  ; fi
# internpro
ARG IP_VER
RUN if [[ -n "${IP_VER}" ]]; then \
  export IP_DL=releases/download/${IP_VER}/internpro-${IP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/${IP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset IP_DL \
  ; fi
# webpro
ARG WP_VER
RUN if [[ -n "${WP_VER}" ]]; then \
  export WP_DL=releases/download/${WP_VER}/webpro-${WP_VER}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset WP_DL \
  ; fi
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
# install data source name (DSN)
#  odbcinst: [Action]i:install [Object]s:data_source [Options]h:user_dsn,f:template_file
#  odbcinst creates ~/.odbc.ini
COPY odbc.ini.test /home/${USERNAME}/
ARG ODBC_INI
RUN if [[ -n "${ODBC_INI}" ]]; then \
  odbcinst -i -s -h -f /home/${USERNAME}/odbc.ini.test \
  ; fi
RUN rm /home/${USERNAME}/odbc.ini.test
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/scripts/entry.sh"]
