FROM ghcr.io/externpro/centos:7
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# yum repositories
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     bzip2 `#firefox` \
     ffmpeg `#browser-video` \
     gtk2 `#old-wx` \
     gtk3 `#firefox,wx` \
     iproute \
     libSM \
     libXt `#firefox` \
     mesa-libGLU \
     make \
     sudo \
     unixODBC \
     wget \
     which \
     xeyes \
     Xvfb \
  && yum clean all
# cmake
RUN export CMK_VER=3.28.3 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-$(uname -s)-$(uname -m).tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# chrome
RUN export CHR_VER=108.0.5359.98 \
  && export CHR_DL=linux/chrome/rpm/stable/$(uname -m)/google-chrome-stable-${CHR_VER}-1.$(uname -m).rpm \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     https://dl.google.com/${CHR_DL} \
  && yum clean all \
  && unset CHR_DL && unset CHR_VER
# firefox
RUN export FOX_VER=102.6.0esr \
  && export FOX_DL=pub/firefox/releases/${FOX_VER}/linux-$(uname -m)/en-US/firefox-${FOX_VER}.tar.bz2 \
  && wget -qO- "https://ftp.mozilla.org/${FOX_DL}" | tar -xj -C /opt/ \
  && ln -s /opt/firefox/firefox /usr/local/bin/firefox \
  && unset FOX_DL && unset FOX_VER
# install database packages from yum repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
RUN yum -y update \
  && yum clean all \
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql https://repo.mysql.com/RPM-GPG-KEY-mysql-2023 \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql \
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
