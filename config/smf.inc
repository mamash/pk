# Default path to hold a ${PKGPATH} like tree of directories to hold the
# SMF manifest/method files
SMFBASE=                ${OVERLAY}/meta

# Per package SMF defines

SMF_USER.postgrey=${POSTGREY_USER}
SMF_GROUP.postgrey=${POSTGREY_GROUP}
SMF_HOME.postgrey=${POSTGREY_DBDIR}

SMF_USER.memcached=${MEMCACHED_USER}

SMF_NAME.ejabberd=jabber:ejabberd
SMF_HOME.ejabberd=${EJABBERD_DB}
SMF_USER.ejabberd=${EJABBERD_USER}
SMF_GROUP.ejabberd=${EJABBERD_GROUP}

SMF_USER.couchdb=${COUCH_USER}
SMF_GROUP.couchdb=${COUCH_GROUP}
SMF_HOME.couchdb=${COUCH_DBDIR}

SMF_NAME.mysql5-server=mysql
SMF_NAME.mysql51-server=mysql
SMF_NAME.mysql55-server=mysql

SMF_NAME.postgresql84-server=postgresql:pg84
SMF_NAME.postgresql90-server=postgresql:pg90
SMF_NAME.postgresql91-server=postgresql:pg91

SMF_NAME.erlang=epmd

SMF_NAME.amavisd-new=amavisd
SMF_USER.amavisd-new=${AMAVIS_USER}
SMF_GROUP.amavisd-new=${AMAVIS_GROUP}

SMF_NAME.courier-imap=courier:imap

SMF_NAME.bind=dns/server

SMF_NAME.net-snmp=sma

SMF_USER.rabbitmq=${RMQ_USER}
SMF_GROUP.rabbitmq=${RMQ_GROUP}
SMF_HOME.rabbitmq=${RMQ_DIR}

SMF_USER.clamav=${CLAMAV_USER}
SMF_GROUP.clamav=${CLAMAV_GROUP}
SMF_HOME.clamav=${CLAMAV_DBDIR}

SMF_NAME.courier-authlib=courier:authlib

SMF_NAME.py27-denyhosts=denyhosts

SMF_NAME.munin-node=munin

SMF_NAME.apache22=apache

SMF_USER.trac=www
SMF_GROUP.trac=www

SMF_USER.varnish=${VRNUSER}
SMF_GROUP.varnish=${VRNGROUP}
