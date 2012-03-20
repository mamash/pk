# Miscellaneous pkgsrc configuration

# Enable vulnerable packages by default
ALLOW_VULNERABLE_PACKAGES=	yes

# Developer mode by default (more checks etc.)
PKG_DEVELOPER=			yes

# Prefer to use existing binary packages, otherwise build deps
DEPENDS_TARGET=			bin-install clean
UPDATE_TARGET=			bin-install clean
