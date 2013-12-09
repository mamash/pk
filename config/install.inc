PKG_DESTDIR_SUPPORT?=		user-destdir

# If an INSTALL file exists under ${OVERLAY}/meta/${PKGPATH} for a package,
# use it to add a post-install script (in addition to any existing ones
# from upstream

.if exists(${OVERLAY}/meta/${PKGPATH}/INSTALL)
INSTALL_TEMPLATES+=		${OVERLAY}/meta/${PKGPATH}/INSTALL
.endif
