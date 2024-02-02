ARG BPROTAG
FROM ghcr.io/externpro/buildpro/rocky85-bld:${BPROTAG}
LABEL maintainer="smanders"
LABEL org.opencontainers.image.source https://github.com/externpro/buildpro
SHELL ["/bin/bash", "-c"]
USER 0
# install database packages from dnf repo
#  https://dev.mysql.com/doc/refman/8.0/en/linux-installation-yum-repo.html
RUN dnf -y update \
  && dnf clean all \
  # https://codingbee.net/uncategorized/yum-error-public-key-for-rpm-is-not-installed
  && wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql https://repo.mysql.com/RPM-GPG-KEY-mysql-2022 \
  && rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-mysql \
  && dnf -y install --setopt=tsflags=nodocs \
     https://repo.mysql.com/mysql80-community-release-el8-9.noarch.rpm \
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
ENTRYPOINT ["/bin/bash", "/usr/local/bpbin/entry.sh"]
