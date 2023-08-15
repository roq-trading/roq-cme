#!/usr/bin/env bash

USER="cmeconfig"
PASSWORD="G3t(0nnect3d"

URL="sftp://sftpng.cmegroup.com"

lftp \
  -u "$USER,$PASSWORD" \
  -e "set ssl:verify-certificate no;" \
  -e "set xfer:clobber on;" \
  $URL << EOM
lcd share/prod
cd /MSGW/Production/Configuration
mget *.xml
cd /SBEFix/Production/Configuration
get config.xml
lcd ../test
cd /MSGW/Cert/Configuration
mget *.xml
cd /SBEFix/Cert/Configuration
get config.xml
lcd ../cert
# maybe...
cd /MSGW/Cert/Configuration
mget *.xml
cd /SBEFix/CertAutoCertPlus/Configuration
get config.xml
bye
EOM
