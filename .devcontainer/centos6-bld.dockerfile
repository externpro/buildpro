FROM ghcr.io/smanders/buildpro/centos6-bld:21.01
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# CRTool
ARG CRTOOL
RUN eval "${CRTOOL}"
ENV PATH=$PATH:/opt/extern/CRTool
# PluginSDK
ARG PLUGINSDK
RUN eval "${PLUGINSDK}"
# internpro
ARG INTERNPRO
RUN eval "${INTERNPRO}"
# webpro
ARG WEBPRO
RUN eval "${WEBPRO}"
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
# needs to run as non-root user
RUN if command -v git-lfs &>/dev/null; then \
  git lfs install \
  ; fi
# run bash script and process the input command
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
