# create non-root user, add to sudoers
ARG USERNAME
ARG USERID
ARG GROUPID
RUN if [ ${USERID:-0} -ne 0 ] && [ ${GROUPID:-0} -ne 0 ]; then \
  export GROUPNAME=$(getent group | grep ${GROUPID} | cut -d: -f1) \
  && if [[ -z ${GROUPNAME} ]]; then groupadd -g ${GROUPID} ${USERNAME}; fi \
  && adduser --uid ${USERID} --gid ${GROUPID} ${USERNAME} \
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
