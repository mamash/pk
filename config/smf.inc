# Default path to hold a ${PKGPATH} like tree of directories to hold the
# SMF manifest/method files
SMFBASE=                ${OVERLAY}/meta

# Per package SMF defines

SMF_USER.postgrey=${POSTGREY_USER}
SMF_GROUP.postgrey=${POSTGREY_GROUP}
SMF_HOME.postgrey=${POSTGREY_DBDIR}

SMF_USER.memcached=${MEMCACHED_USER}

SMF_HOME.ejabberd=${EJABBERD_DB}
SMF_USER.ejabberd=${JABBER_USER}
SMF_GROUP.ejabberd=${JABBER_GROUP}
SMF_USER.spectrum=${JABBER_USER}
SMF_GROUP.spectrum=${JABBER_GROUP}

SMF_USER.couchdb=${COUCH_USER}
SMF_GROUP.couchdb=${COUCH_GROUP}
SMF_HOME.couchdb=${COUCH_DBDIR}

SMF_NAME.mysql-server=mysql

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

SMF_USER.riak=${RIAK_USER}
SMF_GROUP.riak=${RIAK_GROUP}
SMF_HOME.riak=${RIAK_HOME}

SMF_NAME.riakeds=riak
SMF_USER.riakeds=${RIAK_USER}
SMF_GROUP.riakeds=${RIAK_GROUP}
SMF_HOME.riakeds=${RIAK_HOME}

SMF_NAME.zookeeper-server=zookeeper
SMF_USER.zookeeper-server=${ZK_USER}
SMF_GROUP.zookeeper-server=${ZK_GROUP}

SMF_USER.redis=${REDIS_USER}
SMF_GROUP.redis=${REDIS_GROUP}
SMF_HOME.redis=${REDIS_DATADIR}

SMF_NAME.apache-tomcat=tomcat
SMF_USER.apache-tomcat=${TOMCAT_USER}
SMF_GROUP.apache-tomcat=${TOMCAT_GROUP}
