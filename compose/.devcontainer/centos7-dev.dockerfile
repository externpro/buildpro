FROM ghcr.io/smanders/buildpro/centos7-dev:21.12
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# CRTool
ARG CRTOOL
ARG CRWRAP
RUN if [[ -n "${CRTOOL}" && -n "${CRWRAP}" ]]; then \
  mkdir ${EXTERN_DIR}/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/${CRWRAP}/CRTool-${CRWRAP}.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/${CRTOOL}/CRToolImpl-${CRTOOL}.sh" \
  && chmod 755 CRTool*.sh \
  && ./CRTool-${CRWRAP}.sh --prefix=${EXTERN_DIR}/CRTool --exclude-subdir \
  && ./CRToolImpl-${CRTOOL}.sh --prefix=${EXTERN_DIR} --include-subdir \
  && rm CRTool-${CRWRAP}.sh \
  && rm CRToolImpl-${CRTOOL}.sh \
  ; fi
ENV PATH=$PATH:${EXTERN_DIR}/CRTool
# PluginSDK
ARG PLUGINSDK
RUN if [[ -n "${PLUGINSDK}" ]]; then \
  export SDK_DL=releases/download/${PLUGINSDK}/SDLPluginSDK-${PLUGINSDK}-${GCC_VER}-64-Linux.tar.xz \
  && if [[ ${PLUGINSDK} == "v3.0.3.0" ]]; then \
       export SDK_DL=releases/download/${PLUGINSDK}/VantagePluginSDK-${PLUGINSDK}-${GCC_VER}-64-Linux.tar.xz; \
     fi \
  && wget -qO- "https://isrhub.usurf.usu.edu/PluginFramework/SDKSuper/${SDK_DL}" \
   | tar -xJ -C ${EXTERN_DIR} \
  && unset SDK_DL \
  ; fi
# internpro
ARG INTERNPRO
RUN if [[ -n "${INTERNPRO}" ]]; then \
  export IP_DL=releases/download/${INTERNPRO}/internpro-${INTERNPRO}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/${IP_DL}" \
   | tar -xJ -C ${EXTERN_DIR} \
  && unset IP_DL \
  ; fi
ENV INTERNPRO_PATH=${EXTERN_DIR}/internpro-${INTERNPRO}-${GCC_VER}-64-Linux
# webpro
ARG WEBPRO
RUN if [[ -n "${WEBPRO}" ]]; then \
  export WP_DL=releases/download/${WEBPRO}/webpro-${WEBPRO}-${GCC_VER}-64-Linux \
  && if [[ ${WEBPRO} < "20.05.1" ]]; then \
       wget -q "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}.sh" \
       && chmod 755 webpro*.sh \
       && ./webpro-${WEBPRO}-${GCC_VER}-64-Linux.sh --prefix=${EXTERN_DIR} --include-subdir \
       && rm webpro*.sh \
     ; else \
       wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}.tar.xz" \
         | tar -xJ -C ${EXTERN_DIR} \
     ; fi \
  && unset WP_DL \
  ; fi
ENV WEBPRO_PATH=${EXTERN_DIR}/webpro-${WEBPRO}-${GCC_VER}-64-Linux
# create non-root user, add to sudoers
ARG USERNAME
ARG USERID
ARG GROUPID
RUN if [ ${USERID:-0} -ne 0 ] && [ ${GROUPID:-0} -ne 0 ]; then \
  export GROUPNAME=$(getent group ${GROUPID} | cut -d: -f1) \
  && if [[ -z ${GROUPNAME} ]]; then groupadd -g ${GROUPID} ${USERNAME}; fi \
  && useradd --no-log-init --uid ${USERID} --gid ${GROUPID} ${USERNAME} \
  && echo "" >> /etc/sudoers \
  && echo "## dockerfile adds ${USERNAME} to sudoers" >> /etc/sudoers \
  && echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers \
  && unset GROUPNAME \
  ; fi
ENV USER=${USERNAME}
# run container as non-root user from here onwards
# so that build output files have the correct owner
USER ${USERNAME}
# install data source name (DSN)
#  odbcinst: [Action]i:install [Object]s:data_source [Options]h:user_dsn,f:template_file
#  odbcinst creates ~/.odbc.ini
COPY odbc.ini.test /home/${USERNAME}/
RUN odbcinst -i -s -h -f /home/${USERNAME}/odbc.ini.test \
  && rm /home/${USERNAME}/odbc.ini.test
# expose port
EXPOSE 8443
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
