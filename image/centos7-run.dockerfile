FROM ghcr.io/smanders/centos:7
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# yum repositories
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     bzip2 `#firefox` \
     gtk2.x86_64 \
     gtk3.x86_64 `#firefox` \
     iproute \
     libSM.x86_64 \
     libXt.x86_64 `#firefox` \
     mesa-libGLU.x86_64 \
     make \
     sudo \
     unixODBC \
     wget \
     which \
  && yum clean all
# cmake
RUN export CMK_VER=3.21.2 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-Linux-x86_64.tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# chrome
RUN export CHR_VER=88.0.4324.182 \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     https://dl.google.com/linux/chrome/rpm/stable/x86_64/google-chrome-stable-${CHR_VER}-1.x86_64.rpm \
  && yum clean all \
  && unset CHR_VER
# firefox
RUN export FOX_VER=78.7.0esr \
  && wget -qO- "https://ftp.mozilla.org/pub/firefox/releases/${FOX_VER}/linux-x86_64/en-US/firefox-${FOX_VER}.tar.bz2" \
  | tar -xj -C /opt/ \
  && ln -s /opt/firefox/firefox /usr/local/bin/firefox \
  && unset FOX_VER
# install database packages from yum repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     https://repo.mysql.com//mysql80-community-release-el7-3.noarch.rpm \
  && yum -y install --enablerepo=mysql80-community --setopt=tsflags=nodocs \
     mysql-community-client \
     mysql-connector-odbc \
  && yum clean all
# create directories needed by unit tests, autotest
RUN mkdir -p /mnt/mock_midb /mnt/midb /mnt/Plugins /etc/opt/VANTAGE \
  && chmod 777 /mnt/mock_midb /mnt/midb /mnt/Plugins /etc/opt/VANTAGE
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
