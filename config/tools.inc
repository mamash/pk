# pkgsrc include file for tool definitions

BASH=				/usr/bin/bash

CONFIG_SHELL?=			${BASH}
WRAPPER_SHELL?=			${BASH}
COMMAND_SHELL?=			${BASH}

TOOLS_PATH.ksh=			${TOOLS_PLATFORM.ksh}
.if exists(${LOCALBASE}/bin/readelf)
TOOLS_PATH.readelf?=		${LOCALBASE}/bin/readelf
.endif

# Avoid SunOS native openssl
TOOLS_PLATFORM.openssl=		${LOCALBASE}/bin/openssl
# Avoid these platform tools by default
TOOLS_PLATFORM.bison=		#
TOOLS_PLATFORM.yacc=		#
TOOLS_PLATFORM.m4=		#
TOOLS_PLATFORM.strip=		/usr/bin/strip
TOOLS_PLATFORM.tsort=		/usr/bin/tsort

PKG_LD=				/usr/bin/ld

# Use the fortran compiler that's part of our GCC suite
FC=				gfortran
FCPATH=				${LOCALBASE}/bin/${FC}
.if !empty(ABI:M64)
FFLAGS=				-m64
.endif

FALSE=				/usr/bin/false
NOLOGIN=			/usr/bin/false

# Fetch consistently using wget
FETCH_USING=			custom
.if exists(/usr/bin/wget)
# wget on SmartOS
FETCH_CMD=			/usr/bin/wget
.elif exists(/usr/sfw/bin/wget)
# wget on Solaris Nevada and similar
FETCH_CMD=			/usr/sfw/bin/wget
.endif
FETCH_BEFORE_ARGS=		${PASSIVE_FETCH:D--passive-ftp} --no-check-certificate --timeout=60
FETCH_RESUME_ARGS=		-c
FETCH_OUTPUT_ARGS=		-O
