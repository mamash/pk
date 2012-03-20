# Default path to pkgsrc tree
PKGSRCDIR?=     	/opt/pkgsrc

# Default path to source distribution files
DISTDIR?=		/opt/build/distfiles

# Default working directory (where packages are extracted, patched,
# configured, built etc.
WRKOBJDIR?=		/tmp/pkgsrc

# Official support for ad-hoc patches (contains a tree of ${PKGPATH}
# directories for any packages)
LOCALPATCHES=		${OVERLAY}/patches

# Default base for user home directories
HOMEBASE=   		/home

# Default local prefix to install under
DEF_PREFIX=		/opt/local

# If current prefix doesn't match the default prefix, generate a string
# to make the build stand out
.if !empty(DEF_PREFIX)
. if ${LOCALBASE} != ${DEF_PREFIX}
PREFIX_STRING=		-${LOCALBASE:S/\///:S/\//_/}
. endif
.endif

# Default path where binary packages are created
PACKAGES=		/opt/build/packages/${PKGRELEASE}/${COMPILER_STRING}${ABI_STRING}${PREFIX_STRING}
