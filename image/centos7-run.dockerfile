FROM centos:7
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# yum repositories
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     gtk2.x86_64 \
     libSM.x86_64 \
     make \
     sudo \
     unixODBC \
     wget \
  && yum clean all
# cmake
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v3.17.5/cmake-3.17.5-Linux-x86_64.tar.gz" \
  | tar --strip-components=1 -xz -C /usr/local/
# install database connect from yum repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
# install data source name (DSN)
#  odbcinst: [Action]i:install [Object]s:data_source [Options]l:system_dsn,f:template_file
#  odbcinst creates /etc/odbc.ini
COPY odbc.ini.mock /usr/local/src/
RUN yum -y update \
  && yum clean all \
  && yum -y install --setopt=tsflags=nodocs \
     https://repo.mysql.com//mysql80-community-release-el7-3.noarch.rpm \
  && yum -y install --enablerepo=mysql80-community --setopt=tsflags=nodocs \
     mysql-connector-odbc \
  && yum clean all \
  && mkdir -p /mnt/mock_midb \
  && chmod 777 /mnt/mock_midb \
  && odbcinst -i -s -l -f /usr/local/src/odbc.ini.mock \
  && rm /usr/local/src/odbc.ini.mock
