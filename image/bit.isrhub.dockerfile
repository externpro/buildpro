# CRTool
ARG CRTOOL
ARG CRWRAP
RUN if [[ -n "${CRTOOL}" && -n "${CRWRAP}" ]]; then \
  mkdir /opt/extern/CRTool \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRTool/releases/download/${CRWRAP}/CRTool-${CRWRAP}.sh" \
  && wget -q "https://isrhub.usurf.usu.edu/CRTool/CRToolImpl/releases/download/${CRTOOL}/CRToolImpl-${CRTOOL}.sh" \
  && chmod 755 CRTool*.sh \
  && ./CRTool-${CRWRAP}.sh --prefix=/opt/extern/CRTool --exclude-subdir \
  && ./CRToolImpl-${CRTOOL}.sh --prefix=/opt/extern --include-subdir \
  && rm CRTool-${CRWRAP}.sh \
  && rm CRToolImpl-${CRTOOL}.sh \
  ; fi
ENV PATH=$PATH:/opt/extern/CRTool
# PluginSDK
ARG PLUGINSDK
RUN if [[ -n "${PLUGINSDK}" ]]; then \
  export SDK_DL=releases/download/${PLUGINSDK}/SDLPluginSDK-${PLUGINSDK}-${GCC_VER}-64-Linux.tar.xz \
  && if [[ ${PLUGINSDK} == "v3.0.3.0" ]]; then \
       export SDK_DL=releases/download/${PLUGINSDK}/VantagePluginSDK-${PLUGINSDK}-${GCC_VER}-64-Linux.tar.xz; \
     fi \
  && wget -qO- "https://isrhub.usurf.usu.edu/PluginFramework/SDKSuper/${SDK_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset SDK_DL \
  ; fi
# internpro
ARG INTERNPRO
RUN if [[ -n "${INTERNPRO}" ]]; then \
  export IP_DL=releases/download/${INTERNPRO}/internpro-${INTERNPRO}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/smanders/internpro/${IP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset IP_DL \
  ; fi
# webpro
ARG WEBPRO
RUN if [[ -n "${WEBPRO}" ]]; then \
  export WP_DL=releases/download/${WEBPRO}/webpro-${WEBPRO}-${GCC_VER}-64-Linux.tar.xz \
  && wget -qO- "https://isrhub.usurf.usu.edu/webpro/webpro/${WP_DL}" \
   | tar -xJ -C /opt/extern/ \
  && unset WP_DL \
  ; fi
