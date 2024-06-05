FROM ghcr.io/externpro/rockylinux:8.5
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
VOLUME /bpvol
# https://rockylinux.pkgs.org https://rhel.pkgs.org
# AppStream, BaseOS Repositories
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     bzip2 `#firefox` \
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
     Xvfb \
  && dnf clean all
# PowerTools, EPEL Repositories
RUN dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     dnf-plugins-core \
     epel-release \
  && dnf config-manager --set-enabled powertools \
  && dnf config-manager --add-repo=https://negativo17.org/repos/epel-multimedia.repo \
  && dnf -y update \
  && dnf -y install --setopt=tsflags=nodocs \
     ffmpeg `#browser-video` \
     xeyes \
  && dnf clean all
# cmake
RUN export CMK_VER=3.28.3 \
  && export CMK_DL=releases/download/v${CMK_VER}/cmake-${CMK_VER}-$(uname -s)-$(uname -m).tar.gz \
  && wget -qO- "https://github.com/Kitware/CMake/${CMK_DL}" \
  | tar --strip-components=1 -xz -C /usr/local/ \
  && unset CMK_DL && unset CMK_VER
# minimum chrome
RUN export CHR_VER=115.0.5790.110 \
  && export CHR_DL=linux/chrome/rpm/stable/$(uname -m)/google-chrome-stable-${CHR_VER}-1.$(uname -m).rpm \
  && echo "repo_add_once=false" > /etc/default/google-chrome \
  && dnf -y update \
  && dnf clean all \
  && dnf -y install --setopt=tsflags=nodocs \
     https://dl.google.com/${CHR_DL} \
  && dnf clean all \
  && unset CHR_DL && unset CHR_VER
# minimum firefox
RUN export FOX_VER=102.6.0esr \
  && export FOX_DL=pub/firefox/releases/${FOX_VER}/linux-$(uname -m)/en-US/firefox-${FOX_VER}.tar.bz2 \
  && wget -qO- "https://ftp.mozilla.org/${FOX_DL}" | tar -xj -C /opt/ \
  && ln -s /opt/firefox/firefox /usr/local/bin/firefox \
  && unset FOX_DL && unset FOX_VER
# install database packages from dnf repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
RUN dnf -y update \
  && dnf clean all \
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql https://repo.mysql.com/RPM-GPG-KEY-mysql-2023 \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql \
  && dnf -y install --setopt=tsflags=nodocs \
     https://repo.mysql.com//mysql80-community-release-el8-9.noarch.rpm \
  # EL8-based systems: disable the default MySQL Module
  # Warning: native mysql package from platform vendor seems to be enabled, disable before installing packages from repo.mysql.com.
  && dnf module -y disable mysql \
  && dnf -y install --enablerepo=mysql80-community --setopt=tsflags=nodocs \
     mysql-community-client \
     mysql-connector-odbc \
  && dnf clean all
# create directories needed by unit tests, autotest
RUN mkdir -p /mnt/mock_midb /mnt/midb /mnt/Plugins /etc/opt/VANTAGE \
  && chmod 777 /mnt/mock_midb /mnt/midb /mnt/Plugins /etc/opt/VANTAGE
# copy from local into image
COPY scripts/ /usr/local/bpbin
COPY git-prompt.sh /etc/profile.d/
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
