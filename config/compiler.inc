# We prefer to use a native GCC (i.e. not one from upstream pkgsrc)
USE_NATIVE_GCC?=	no

GCC_USE_RUNTIME?=	yes
GCC_MAJOR_MINOR?=	47

# We use x86_64 to identify 64bit packages
# (upstream pkgsrc still uses i386 for both Intel archs on SunOS)
.if defined(ABI) && ${ABI} == 64
MACHINE_ARCH=		x86_64
.endif

ABI_STRING=		_${ABI}

# Optimize to -O2 by default
CFLAGS+=		-O2

# Ignore out-of-scope definition errors
CXXFLAGS+=		-fpermissive

# Parallel make jobs
MAKE_JOBS=		8

# The Sun linker on Illumos based SunOS flavors is much more reliable
# with -shared rather than -G. Sadly -shared is not supported on
# e.g. Nevada or older Solaris releases
BUILDLINK_TRANSFORM+=	rename:-G:-shared

# Generate a string to identify the compiler setup (e.g. 'gcc46')
.if !empty(PKGSRC_COMPILER)
. if ${PKGSRC_COMPILER} == sunpro
SUNWSPROBASE?= 		/opt/SUNWspro
COMPILER_VERSION!=	basename ${SUNWSPROBASE} |\
			  sed 's/[a-zA-Z ]*\([0-9]*\).*/\1/'
. elif ${PKGSRC_COMPILER} == gcc
COMPILER_VERSION=	${CC_VERSION:S/-/ /:[2]:C/\.[0-9]*$//:S/.//}
. endif
.endif
COMPILER_STRING=	${PKGSRC_COMPILER}${COMPILER_VERSION}
