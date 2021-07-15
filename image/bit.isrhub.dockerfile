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
  export WP_DL=releases/download/${WEBPRO}/webpro-${WEBPRO}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}" \
   | tar -xJ -C ${EXTERN_DIR} \
  && unset WP_DL \
  ; fi
ENV WEBPRO_PATH=${EXTERN_DIR}/webpro-${WEBPRO}-${GCC_VER}-64-Linux
