# Let us identify SmartOS (vs. vanilla Solaris)
OS!=			echo `uname -s`
.if ${OS} == SunOS
SUNOS_REL!=		echo `${UNAME} -v`
SUNOS_FLAVOR=		${SUNOS_REL:S/_/ /:[1]}
SUNOS_VER=  		${SUNOS_REL:S/_/ /:[2]}
. if ${SUNOS_FLAVOR} == joyent
SUNOS_VER=		147
. endif
.endif

# Generally prefer everything built from pkgsrc
PREFER_PKGSRC=			yes
PREFER.openssl=			pkgsrc
PREFER.gettext=			pkgsrc

# Try to use a native instance for these though
PREFER_NATIVE=			solaris-pam sun-libc libuuid terminfo
PREFER_NATIVE_PTHREADS=		yes

PTHREAD_AUTO_VARS=		yes

.if ${OPSYS} == "SunOS" && exists(/usr/jdk)
# Solaris or Nevada, use a native Java
PREFER.sun-jre6=		native
PREFER.sun-jdk6=		native
PKG_JVM_DEFAULT=		sun-jdk6
PKG_JAVA_HOME=			/usr/jdk/latest
JAVA_BINPREFIX=			/usr/bin
.else
# Must be SmartOS, need to build our own Java
PKG_JVM_DEFAULT=		sun-jdk6
.endif

.if !exists(/usr/include/audio/au.h)
# Where the Sun audio include files are not installed on the OS
SUN_NO_AUDIO=			yes
.endif
