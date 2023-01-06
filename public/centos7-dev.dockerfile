FROM ghcr.io/smanders/buildpro/centos7-bld:23.01
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/smanders/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# install database packages from yum repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
RUN yum -y update \
  && yum clean all \
  # https://codingbee.net/uncategorized/yum-error-public-key-for-rpm-is-not-installed
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql https://repo.mysql.com/RPM-GPG-KEY-mysql-2022 \
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
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
