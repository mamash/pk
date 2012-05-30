# Default pkgsrc-wide options possibly employed by packages
PKG_DEFAULT_OPTIONS=		-x11 dtrace -doc unicode inet6

# Package specific build options
PKG_OPTIONS.ruby=		-ruby-build-ri-db
PKG_OPTIONS.apache=		suexec apache-shared-modules
PKG_OPTIONS.apr-util=		ldap db4
PKG_OPTIONS.postfix=		pcre sasl tls sqlite
PKG_OPTIONS.proftpd=		tls pam
PKG_OPTIONS.subversion=		#
PKG_OPTIONS.django=		mysql pgsql sqlite
PKG_OPTIONS.courier-authlib=	bdb pam
PKG_OPTIONS.ImageMagick=	-wmf
PKG_OPTIONS.lynx=		-curses ncurses
PKG_OPTIONS.openldap-server=	sasl bdb
PKG_OPTIONS.openldap-client=	sasl
PKG_OPTIONS.ffmpeg=		xvid x264 theora tools opencore-amr
.if defined(PKGPATH) && ${PKGPATH} == "mail/dovecot"
PKG_OPTIONS.dovecot=		pam sqlite dovecot-sieve dovecot-managesieve
.else
PKG_OPTIONS.dovecot=		pam sqlite
.endif
PKG_OPTIONS.mutt=		-curses ncurses
PKG_OPTIONS.groff=		-groff-docs
PKG_OPTIONS.mysql5-client=	-ssl
.if defined(PKGPATH) && !empty(PKGPATH:Mmysql51)
PKG_OPTIONS.mysql5=		-embedded-server -mysqlmanager -ndb-cluster -pstack
.else
PKG_OPTIONS.mysql5=		-embedded-server -ndb-cluster
.endif
PKG_OPTIONS.lighttpd=		lua bzip ldap
PKG_OPTIONS.mc=			-slang ncurses
PKG_OPTIONS.dbus=		#
PKG_OPTIONS.sympa=		mysql pgsql
PKG_OPTIONS.nagios-nrpe=	ssl nagios-nrpe-args
PKG_OPTIONS.gdb6=		-gdb6-tui
PKG_OPTIONS.SDL=		-arts -esound -nas -pulseaudio noaudio
PKG_OPTIONS.mencoder=		-dts dv -dvdread -dvdnav gif jpeg lame mad -mlib png theora vorbis x264 xvid -mplayer-runtime-cpudetection
PKG_OPTIONS.elinks=		elinks-root-exec
PKG_OPTIONS.gmp=		-gmp-fat
PKG_OPTIONS.gtk2=		x11
PKG_OPTIONS.wxGTK28=		unicode
PKG_OPTIONS.ghostscript=	#
PKG_OPTIONS.erlang=		unixodbc
PKG_OPTIONS.tidy=		-doc
PKG_OPTIONS.x264-devel=		-threads
PKG_OPTIONS.screen=		ncurses
PKG_OPTIONS.ap-modsecurity2=	curl
PKG_OPTIONS.pkgin=		gzip-summary
PKG_OPTIONS.postgresql84-server=dtrace
PKG_OPTIONS.postgresql90-server=dtrace
PKG_OPTIONS.sphinx-search=	mysql pgsql
PKG_OPTIONS.spamprobe=		bdb
PKG_OPTIONS.pulseaudio=		-avahi
PKG_OPTIONS.libsndfile=		-sun
PKG_OPTIONS.scmgit=		-scmgit-gui -python
PKG_OPTIONS.curl=		libssh2
PKG_OPTIONS.webalizer=		geoip
PKG_OPTIONS.openssh=		hpn-patch
PKG_OPTIONS.nginx=		dav status realip uwsgi memcache
PKG_OPTIONS.php-sqlite=		utf8
PKG_OPTIONS.graphviz=		-gtk pangocairo x11
PKG_OPTIONS.roundcube=		sqlite
PKG_OPTIONS.spidermonkey=	-threads unicode
PKG_OPTIONS.cairo=		x11
PKG_OPTIONS.libpurple=		-gstreamer -farsight -avahi -dbus -gnome
PKG_OPTIONS.sysbench=		mysql pgsql
PKG_OPTIONS.netcat6=		inet6
PKG_OPTIONS.sudo=		pam
PKG_OPTIONS.p5-HTML-Mason=	modperl fcgi
PKG_OPTIONS.heimdal=		kerberos-prefix-cmds
PKG_OPTIONS.mpg123=		mpg123-with-fpu -mpg123-fifo -mpg123-x86-dither
PKG_OPTIONS.libarchive=		-ssl -lzma
PKG_OPTIONS.tftp-hpa=		remap
PKG_OPTIONS.cups=		-dbus -dnssd -kerberos -libusb -slp -ghostscript 
PKG_OPTIONS.libthrift=  	java php ruby
PKG_OPTIONS.rsyslog=		file
PKG_OPTIONS.openoffice3=	lang-all -xulrunner -gtk2
PKG_OPTIONS.sendmail=		tls
PKG_OPTIONS.riak=		innostore
PKG_OPTIONS.sqlrelay=		mysql sqlite oracle
PKG_OPTIONS.php=		readline
PKG_OPTIONS.MesaLib=		dri
PKG_OPTIONS.wireshark=		-gtk2
PKG_OPTIONS.milter-greylist=	postfix-milter
PKG_OPTIONS.gcc47=		-gcc-go
PKG_OPTIONS.doxygen=		-latex
PKG_OPTIONS.openvpn=		pam

# Package specific configuration options

# Build a modular (pkgsrc based) X11 chain
X11_TYPE=			modular

# GCC
# Prefer to build Java support into GCC (saves *lots* of time and disk)
BUILD_JAVA=			no

# GNU tools
# Do not use prefixes for GNU tools, use 'n' for the NetBSD variants though
GNU_PROGRAM_PREFIX= 		# empty
GTAR_PROGRAM_PREFIX=  		${GNU_PROGRAM_PREFIX}
GROFF_COMMAND_PREFIX=		${GNU_PROGRAM_PREFIX}
NBPAX_PROGRAM_PREFIX=		n

# curses
CURSES_DEFAULT=			ncurses

# Apache
APACHE_SUEXEC_DOCROOT=		/home
APACHE_SUEXEC_CONFIGURE_ARGS=	--with-suexec-userdir=web/public
APACHE_MODULES=			all-shared

# Courier
AUTHDAEMONVAR=			/var/authdaemon

# Mailman
MAILMAN_CGIEXT=			# empty
MAILMAN_USER=			mailman
MAILMAN_GROUP=			mailman
MAILMAN_MAILGROUP=		nobody

# MySQL
MYSQL_VERSION_DEFAULT=		55
MYSQL_DATADIR=			/var/mysql
MYSQL_USER=			mysql
MYSQL_GROUP=			mysql
MYSQL_CHARSET=			utf8
MYSQL_COLLATION=		utf8_general_ci

# PostgreSQL
PGSQL_VERSION_DEFAULT=		91
PGHOME=				/var/pgsql
PGUSER=				postgres
PGGROUP=			postgres

# Python
PYTHON_VERSION_DEFAULT=		27

# PHP
PHP_VERSION_DEFAULT=		53
PHP_MYSQL_DRIVER=		mysqlnd

# Sympa
SYMPA_USER=			sympa
SYMPA_GROUP=			sympa
SYMPA_VARBASE=			/var/sympa

# Dspam
DSPAM_DELIVERY_AGENT=		maildrop

# Amavis
AMAVIS_USER=			amavis
AMAVIS_GROUP=			amavis

#eruby
ERUBY_DEFAULT_CHARSET=		utf8

# ruby
RUBY_VERSION_DEFAULT=		18

#rails
RUBY_RAILS_DEFAULT=		32

#nginx
NGINX_PIDDIR=			/var/db/nginx
NGINX_USER=			www
NGINX_GROUP=			www

#prosody
PROSODY_RUN=			/var/db/prosody

#redis
REDIS_USER?=			redis
REDIS_GROUP?=			redis
REDIS_PIDDIR=			/var/db/redis

#postfix
POSTFIX_USER=			postfix
POSTFIX_GROUP=			postfix

#munin
MUNIN_RUN=			/var/munin/run

#bind
BIND_DIR=			/var/named

#jabber related packages
JABBER_USER=			ejabberd
JABBER_GROUP=			ejabberd
EJABBERD_USER=			${JABBER_USER}
EJABBERD_GROUP=			${JABBER_GROUP}

#spectrum
SPECTRUM_RUN=			/var/db/spectrum
SPECTRUM_DB=			/var/db/spectrum
SPECTRUM_LOG=			/var/log/spectrum

#zookeeper
ZK_RUN=				/var/db/zookeeper
