# This file defines 
# If an ALTERNATIVES file is found for a package under
# ${OVERLAY}/meta/${PKGPATH}, it is used. Raise a conflict if the package
# already has such file, because pkgsrc doesn't support multiple sources yet

.if exists(${OVERLAY}/meta/${PKGPATH}/ALTERNATIVES)
. if exists(${PKGSRCDIR}/${PKGPATH}/ALTERNATIVES)
PKG_FAIL_REASON+=		"Conflict between package's ALTERNATIVES files!"
. else
ALTERNATIVES_SRC=		${OVERLAY}/meta/${PKGPATH}/ALTERNATIVES
. endif
.endif
